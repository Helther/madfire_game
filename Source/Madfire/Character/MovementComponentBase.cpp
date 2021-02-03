// Fill out your copyright notice in the Description page of Project Settings.

#include "MovementComponentBase.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "CharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PhysicsVolume.h"
#include "Rules/Utils.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"


const FName FTransitionSectionNames::Start = "transition_start";
const FName FTransitionSectionNames::End = "transition_finish";

UBaseMovementComponent::UBaseMovementComponent(const class FObjectInitializer &ObjectInitilizer) :
    Super(ObjectInitilizer)
{
    bNotifyApex = true;
    bCanWalkOffLedgesWhenCrouching = true;
    //////////////////////////////////////Debug///////////////////////////
    MaxWalkSpeedDefault = 250;
    MaxWalkSpeedCrouchedDefault = 150;
    MaxRunSpeedDefault = 500;
    MaxRunSpeedCrouchedDefault = 300;
    MaxSprintSpeedDefault = 700;
    MaxClimbRotationAngle = 70;
    MaxVaultLedgeSize = 85;
    MaxStepHeight = 40;
    LedgeTraceLength = 70;
    LocationHorizOffset = 3;
    LocationVertOffset = 3;
    TransitionMoveSpeed = 0.4f;
    TransitionRotationSpeed = 0.5f;
    MoveAbilitiesCurrent = MoveAbilitiesDefault = 0xff;
    SprintConsumeTime = 0.5;

    JumpWallZVelocityBase = 400;
    JumpWallXYVelocityBase = 200;
    MinFallHeightThresholdBase = 100;
    MaxfallHeightThresholdBase = 500;
    
    AirControlBase = 0.05f;
    JumpZVelocityBase = 400.f;
    BrakingDecelerationFallingBase = 0.f;// пока не работает
    FallingLateralFrictionBase = 0.f; // пока не работает
    JumpMaxHoldTimeBase = 1.f;
    MaxAccelerationBase = 2048.f;
    BrakingDecelerationWalkingBase = 2048.f;
    BrakingFrictionBase = 2.f;
    AnimPlayRateBase = 1.f;
    MaxOverweightMoveFactorBase = 1.2f;

    JumpWallZVelocity = JumpWallZVelocityBase;
    JumpWallXYVelocity = JumpWallXYVelocityBase;
    MinFallHeightThreshold = MinFallHeightThresholdBase;
    MaxfallHeightThreshold = MaxfallHeightThresholdBase;
    AirControl = AirControlBase;
    JumpZVelocity = JumpZVelocityBase;
    BrakingDecelerationFalling = BrakingDecelerationFallingBase;
    FallingLateralFriction = FallingLateralFrictionBase;
    MaxAcceleration = MaxAccelerationBase;
    BrakingDecelerationWalking = BrakingDecelerationWalkingBase;
    BrakingFriction = BrakingFrictionBase;
    AnimPlayRate = AnimPlayRateBase;
    MaxOverweightMoveFactor = MaxOverweightMoveFactorBase;

    FallDamageScale = 0.25f;
    SpeedScale = 0.25f;
    AccelerationScale = 0.25f;
    AnimPlayRateScale = 0.25f;
    FrictionScale = 0.25f;

    abilityMod = 1.f;
    skillMod = 1.f;
    weightMod = 1.f;
    weightFactorBalanceMod = 10.f;
    

    //MoveAbilitiesCurent &= ~ (EMoveAbilities::CanCrouch | EMoveAbilities::CanJump);

    //////////////////////////////////////End Degbug//////////////////////
    /*
    bUseControllerDesiredRotation
    bOrientRotationToMovement
    CrouchedHalfHeight
    JumpZVelocity
    AirControl
    MaxCustomMovementSpeed
    */
}

// вызывать при изменении модификатора
void UBaseMovementComponent::CalculateMaxSpeeds(float SpeedModifier)
{
    if(!CanMove())// скорости на которые не влияют модификаторы
    {
        MaxWalkSpeed = 0;
        MaxWalkSpeedCrouched = 0;
        SpeedModifier = 0;
    }
    else
    {
        MaxWalkSpeed = MaxWalkSpeedDefault;
	    MaxWalkSpeedCrouched = MaxWalkSpeedCrouchedDefault;
    }
    // скорости которые можно модифицировать и бафать
    MaxCustomMovementSpeed = MaxWallClimbSpeedDefault * SpeedModifier;
	MaxSwimSpeed = MaxSwimSpeedDefault * SpeedModifier;
	MaxFlySpeed = MaxFlySpeedDefault * SpeedModifier;
    if(Character->GetCombatComponent()->IsWeaponDrawn)
        CalculateMaxCombatSpeeds(SpeedModifier);
    else
    {
        MaxRunSpeed = MaxRunSpeedDefault * SpeedModifier;
        MaxSprintSpeed = MaxSprintSpeedDefault * SpeedModifier;
        MaxRunSpeedCrouched = MaxRunSpeedCrouchedDefault * SpeedModifier;
    }
}

void UBaseMovementComponent::CalculateMaxCombatSpeeds(float SpeedModifier)
{
    if(Character->GetCombatComponent()->IsBlocking)
        CalculateMaxBlockingSpeed(SpeedModifier);
    else
    MaxRunSpeed = Character->GetCombatComponent()->GetCurrentCombatStance()->
        MaxCombatRunSpeed * SpeedModifier;
    MaxSprintSpeed = Character->GetCombatComponent()->GetCurrentCombatStance()->
        MaxCombatSprintSpeed * SpeedModifier;
    MaxRunSpeedCrouched = Character->GetCombatComponent()->GetCurrentCombatStance()->
        MaxCombatCrouchedSpeed * SpeedModifier;
}

void UBaseMovementComponent::CalculateMaxBlockingSpeed(float SpeedModifier)
{
    MaxRunSpeed = Character->GetCombatComponent()->GetCurrentCombatStance()->
        MaxBlockSpeed * SpeedModifier;
}

void UBaseMovementComponent::CalculateMaxCastingSpeed(float SpeedModifier)
{
    MaxRunSpeed = Character->GetCombatComponent()->GetCurrentCombatStance()->
        MaxCastingSpeed * SpeedModifier;
}

FTraceSpawnParams UBaseMovementComponent::GetTraceSpawnParams(bool IsLow)
{
    const FVector CharLocation = Character->GetActorLocation();
    const FRotator CharRotation = Character->GetActorRotation();
    FTraceSpawnParams sParamsH;
    FQuat Rotation(0, 0, 0, 1);
    Rotation = Character->GetActorRotation().Quaternion();
    sParamsH.Rotation = Rotation;
    auto LedgeTraceHeight = Character->GetDefaultHalfHeight();
    auto crouchHeightDiff = Character->GetDefaultHalfHeight() - CrouchedHalfHeight;
    if(IsLow)
    {
        if(IsCrouching())
            sParamsH.Start = sParamsH.End = CharLocation +
                FVector(0,0,crouchHeightDiff) + HorizLedgeTraceOffset;
        else
            sParamsH.Start = sParamsH.End = CharLocation + HorizLedgeTraceOffset;
        sParamsH.End += CharRotation.Vector() * LedgeTraceLength;
    }
    else
    {
        if(IsCrouching())
            sParamsH.Start = sParamsH.End = CharLocation +
                                            FVector(0,0,crouchHeightDiff +
                                            LedgeTraceHeight) + HorizLedgeTraceOffset;
        else
            sParamsH.Start = sParamsH.End = CharLocation + 
                                    FVector(0,0,LedgeTraceHeight) +
                                     HorizLedgeTraceOffset;
        sParamsH.End += CharRotation.Vector() * LedgeTraceLength;
    }
    return sParamsH;
}

bool UBaseMovementComponent::CanVaultOrClimb(bool IsLow)
{
    if(IsRootOrTeleport() || !CanMove())
        return false;
    UE_LOG(LogTemp,Warning, TEXT("CanVault or climb %s"), IsLow ? TEXT("low") : TEXT("high"));
    const FRotator CharRotation = Character->GetActorRotation();
    //setting spawn parameters
    FTraceSpawnParams sParamsH = GetTraceSpawnParams(IsLow);
    //get max distance allowed for parkour from character's feet
    float maxHeight = Character->GetDefaultHalfHeight() * 2.5;
    float minHeight = Character->GetDefaultHalfHeight();
    if(IsLow)
    {
        maxHeight = Character->GetDefaultHalfHeight();
        minHeight = MaxStepHeight;
    }
    //horiz and vert trace to find ledge location
    auto ledgeLocRot = GetClimbStartLoc(sParamsH, minHeight, maxHeight);
    if(ledgeLocRot.Get() == nullptr)
        return false;
    // проверка на возможность волта
    if(VaultSpaceCheck(*ledgeLocRot.Get()))
    {
        UE_LOG(LogTemp,Warning, TEXT("CanVaultOrClimb: trying vault %s"), 
                IsLow ? TEXT("low") : TEXT("high"));
        if(DoVault(*ledgeLocRot.Get(), IsLow))
            return true;
    }
    UE_LOG(LogTemp,Warning, TEXT("CanVaultOrClimb: doing climb %s"), 
        IsLow ? TEXT("low") : TEXT("high"));
    if(DoClimbLedge(*ledgeLocRot.Get(), IsLow))
        return true;
    return false;
}

bool UBaseMovementComponent::DoVault(const StartClimbData& LocationData, 
                                    bool IsLow)
{
    // vector in the direction opposite to the wall's normal
    const auto res = FQuat(FRotator(0, LocationData.Rotation.Yaw, 0)) * FQuat(FRotator(0, 180, 0));
    const FRotator wallNormalDir(res.Rotator());
    //check for available space at the position before anim
    float charPelvisheight = Character->GetActorLocation().Z;;
    if(IsCrouching())
        charPelvisheight = Character->GetActorLocation().Z + 
            Character->GetDefaultHalfHeight() - CrouchedHalfHeight;
    // коррекция по высоте
    FVector VaultAnimationStartLocation = LocationData.Location +
                                        FVector::UpVector * LocationVertOffset +
                                        wallNormalDir.Vector() *
                                        (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 
                                        LocationHorizOffset);
    if(LocationData.Location.Z < charPelvisheight)
        VaultAnimationStartLocation.Z = charPelvisheight +
                                        LocationVertOffset;
    const FVector VaultAnimationEndLocation = VaultAnimationStartLocation +
                                             FRotator(0, LocationData.Rotation.Yaw, 0).Vector() *
                                            (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() * 2 + 
                                            LocationHorizOffset + MaxVaultLedgeSize);// todo temp

    if(IsEnoughSpace(VaultAnimationStartLocation) == EClimbTraceResult::SuccessStand &&
        Character->GetAttributesComponent()->Stamina->GetValue() > 0)
    {
        Character->GetAttributesComponent()->Stamina->Consume(StaminaCostValues.Vault);
        if(IsLow)
         //todo сделать на время выполнения анимации капсуле размер капсулы в присяде
            TransitionStart(VaultAnimationStartLocation, LocationData.Rotation,
                                 VaultMontageLow, AnimTypes::Parkour,
                                 ECustomMovementMode::ClimbingLedge);
        else
            TransitionStart(VaultAnimationStartLocation, LocationData.Rotation,
                                 VaultMontageHigh, AnimTypes::Parkour,
                                 ECustomMovementMode::ClimbingLedge);
        return true;
    }
    return false;
}

bool UBaseMovementComponent::DoClimbLedge(const StartClimbData& LocationData, bool IsLow)
{
    ///TODO add stamina calculation and usage
    // vector in the direction opposite to the wall's normal
    const auto res = FQuat(FRotator(0, LocationData.Rotation.Yaw, 0)) * FQuat(FRotator(0, 180, 0));
    const FRotator wallNormalDir(res.Rotator());
    // check for available space at the position before anim
    auto ClimbAnimHorizDistance = 50;///TODO temp длина пути анимации
    float charPelvisheight = Character->GetActorLocation().Z;
    if(IsCrouching())
        charPelvisheight = Character->GetActorLocation().Z + Character->GetDefaultHalfHeight() - 
                            CrouchedHalfHeight;
    // коррекция по высоте
    FVector ClimbledgeAnimationStartLocation = LocationData.Location +
                                        FVector::UpVector * LocationVertOffset
                                        + wallNormalDir.Vector() *
                                        (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 
                                        LocationHorizOffset);
    if(LocationData.Location.Z < charPelvisheight)
        ClimbledgeAnimationStartLocation.Z = charPelvisheight +
                                        LocationVertOffset;
    FVector ClimbledgeAnimationEndLocation = ClimbledgeAnimationStartLocation + 
            FVector(0,0,Character->GetDefaultHalfHeight()) +
            LocationData.Rotation.Vector() *
            (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + LocationHorizOffset + 
            ClimbAnimHorizDistance);// todo temp
    ClimbledgeAnimationEndLocation.Z = LocationData.Location.Z + Character->GetDefaultHalfHeight() +
                                        LocationVertOffset;// todo temp
    if(IsCrouching())// todo temp
        ClimbledgeAnimationEndLocation.Z += -Character->GetDefaultHalfHeight() + CrouchedHalfHeight;
    // check for available space at the start position before anim
    if(Character->GetAttributesComponent()->Stamina->GetValue() <= 0)
        return false;
    switch(IsEnoughSpace(ClimbledgeAnimationStartLocation, ClimbledgeAnimationEndLocation))
    {
        case EClimbTraceResult::SuccessStand:
        {
            Character->GetAttributesComponent()->Stamina->Consume(StaminaCostValues.Climb);
            if(IsLow)
                TransitionStart(ClimbledgeAnimationStartLocation, LocationData.Rotation,
                                 ClimbLedgeMontageLow, AnimTypes::Parkour,
                                 ECustomMovementMode::ClimbingLedge);
            else
                TransitionStart(ClimbledgeAnimationStartLocation, LocationData.Rotation,
                                 ClimbLedgeMontageHigh, AnimTypes::Parkour,
                                 ECustomMovementMode::ClimbingLedge);
            return true;
        }
        case EClimbTraceResult::SuccessCrouch:
        {
            Character->GetAttributesComponent()->Stamina->Consume(StaminaCostValues.Climb);
            //if(!IsCrouching())
                //bWantsToCrouch = true; // todo temp
            if(IsLow)
                TransitionStart(ClimbledgeAnimationStartLocation, LocationData.Rotation,
                                 ClimbLedgeCrouchedMontageLow,  AnimTypes::Parkour,
                                 ECustomMovementMode::ClimbingLedge);
            else
                TransitionStart(ClimbledgeAnimationStartLocation, LocationData.Rotation,
                                 ClimbLedgeCrouchedMontageHigh,  AnimTypes::Parkour,
                                 ECustomMovementMode::ClimbingLedge);
            return true;
        }
        default:
            return false;
    }
}

bool UBaseMovementComponent::VaultSpaceCheck(const StartClimbData& LedgeData)
{
    FQuat Rotation(FRotator(0,LedgeData.Rotation.Yaw,90));
    FHitResult vaultHit;
    FTraceSpawnParams sParams;
    sParams.Rotation = Rotation;
    //trace на наличие места для капсулы сидя во время волта
    const FRotator WallNormalDir(0,LedgeData.Rotation.Yaw,0);
    sParams.Start = LedgeData.Location + FVector(0, 0, CrouchedHalfHeight + LocationVertOffset);
    sParams.End = sParams.Start + WallNormalDir.Vector() * MaxVaultLedgeSize;
    UTraceManager::CapsuleTraceSingleByChannel(Character->GetWorld(),
                                               CrouchedHalfHeight,
                                               Character->GetCapsuleComponent()->GetScaledCapsuleRadius(),
                                               params,
                                               sParams,
                                               vaultHit,
                                               EWallChannel);
    if(vaultHit.GetActor() == nullptr)// проверка достаточного места за преграждением для волта
    {
        float traceCapsuleRadius = 10;
        const float traceMod = 0.5f;
        sParams.Start = sParams.End += WallNormalDir.Vector() * traceCapsuleRadius;
        sParams.End.Z -= Character->GetDefaultHalfHeight() * traceMod;
        UTraceManager::CapsuleTraceSingleByChannel(Character->GetWorld(), traceCapsuleRadius,
                                        traceCapsuleRadius, params, sParams, vaultHit,
                                        EWallChannel);
        if(vaultHit.GetActor() == nullptr)
            return true;
    }
    UE_LOG(LogTemp,Warning, TEXT("VaultSpaceCheck error: fail"))
    return false;
}

TSharedPtr<StartClimbData> UBaseMovementComponent::GetClimbStartLoc(const FTraceSpawnParams& StartTrace,
                                                const float& MinHeight,
                                                const float& MaxHeight,
                                                bool CheckAngle)
{
    auto CharRotation = Character->GetActorRotation();
    auto FinalLocation = MakeShared<StartClimbData>();
    auto charBottomHeight = Character->GetActorLocation().Z - Character->GetDefaultHalfHeight();
    if(IsCrouching())
        charBottomHeight = Character->GetActorLocation().Z - CrouchedHalfHeight;
    FHitResult HitWallH;
    FTraceSpawnParams sParamsH = StartTrace;
    auto traceCapsuleHalfHeight = (MaxHeight - MinHeight) / 2;
    int NumOfIterations = 1;
    int MaxNumOfIter = 10; // todo temp
    auto currentTraceHeight = sParamsH.Start.Z;
    // проверка трейсами на подходящий край для зацепа за стену
    for(; NumOfIterations <= MaxNumOfIter; ++NumOfIterations)
    {
        UTraceManager::CapsuleTraceSingleByChannel(Character->GetWorld(), 
                                                   Character->GetCapsuleComponent()->GetScaledCapsuleRadius(),
                                                   traceCapsuleHalfHeight, params, sParamsH, HitWallH,
                                                   EWallChannel, false);
        // нашел ли стену
        if(HitWallH.GetActor() == nullptr)
            return nullptr;
        // подходящий ли угол между персом и стеной
        if(CheckAngle && !checkRelativeRotation(HitWallH.ImpactNormal.Rotation().Yaw))
            return nullptr;
        // ограничение по числу итераций
        if((MinHeight > currentTraceHeight) && (currentTraceHeight > MaxHeight))
        {
            UE_LOG(LogTemp,Warning, TEXT("GetClimbStartLoc error: exceeded height limits"))
            return nullptr;
        }
        // проверим есть ли место для капсулы перса для начала анимации
        const FRotator wallNormalDir(0,HitWallH.ImpactNormal.Rotation().Yaw,0);
        FTraceSpawnParams sParamsSPace;
        sParamsSPace.Start = sParamsSPace.End = HitWallH.ImpactPoint + wallNormalDir.Vector() * 
                            LocationHorizOffset;
        sParamsSPace.Start.Z = charBottomHeight + MaxHeight;
        sParamsSPace.End.Z = charBottomHeight + MinHeight + LocationVertOffset;
        FHitResult HitSpace;
        UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params,
                                            sParamsSPace, HitSpace,
                                            EWallChannel, false);
        if(HitSpace.GetActor() == nullptr)
            break;
        else
            UE_LOG(LogTemp,Warning, TEXT("GetClimbStartLoc error: not enough space"))
        if (NumOfIterations == MaxNumOfIter)
        {
            UE_LOG(LogTemp, Warning, TEXT("GetClimbStartLoc error: reached max num of iterations"))
            break;
        }
        // обновление параметров трейса для новой точки
        currentTraceHeight = HitWallH.ImpactPoint.Z - charBottomHeight;
        if(FMath::Abs(currentTraceHeight - MaxHeight) >= FMath::Abs(currentTraceHeight - MinHeight))
            traceCapsuleHalfHeight = FMath::Abs(currentTraceHeight-MinHeight);
         else
            traceCapsuleHalfHeight = FMath::Abs(currentTraceHeight-MaxHeight);
        auto currentDistanceToWall = FVector::Dist(HitWallH.ImpactPoint, sParamsH.Start);
        sParamsH.Start = sParamsH.End = HitWallH.ImpactPoint;
        sParamsH.Start += wallNormalDir.Vector() * currentDistanceToWall;
        HitWallH.Reset();
    }
    UE_LOG(LogTemp,Warning, TEXT("GetClimbStartLoc: Number of steps %d"),NumOfIterations);
    // vert trace линией вниз для нахождения вертикальной точки на выступе
    const auto res = FQuat(FRotator(0, HitWallH.ImpactNormal.Rotation().Yaw, 0)) * FQuat(FRotator(0, 180, 0));
    const FRotator offsetDir(res.Rotator());
    FTraceSpawnParams sParamsV;
    float heightAllowance = 10;// todo temp
    sParamsV.Start = sParamsV.End = HitWallH.ImpactPoint + offsetDir.Vector();
    sParamsV.Start.Z = charBottomHeight + MaxHeight + heightAllowance;
    sParamsV.End.Z = charBottomHeight + MinHeight;
    FHitResult HitWallV;
    UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params,
                                            sParamsV, HitWallV,
                                            EWallChannel, false);
    // попал ли вертикальный трейс
    if(HitWallV.GetActor() == nullptr || MinHeight > (HitWallV.ImpactPoint.Z - charBottomHeight)
    || (HitWallV.ImpactPoint.Z - charBottomHeight) > MaxHeight)
        return nullptr;
    // установка финальных значений
    FinalLocation.Get().Location = HitWallH.ImpactPoint;
    FinalLocation.Get().Location.Z = HitWallV.ImpactPoint.Z;
    FinalLocation.Get().Rotation = offsetDir;
    return FinalLocation;
}

bool UBaseMovementComponent::checkRelativeRotation(const float WallNormalYaw)
{
    FVector charsV = FRotator(0, Character->GetActorRotation().Yaw, 0).Vector();
    charsV.Normalize();
    FVector wallsV = FRotator(0, WallNormalYaw, 0).Vector();
    wallsV.Normalize();
    auto resAngle = FMath::Abs(acos(FVector::DotProduct(charsV, wallsV)) * 180.f / PI - 180.f);
    const bool res = resAngle < MaxClimbRotationAngle;
    if(!res)
        UE_LOG(LogTemp,Warning, TEXT("checkRelativeRotation error: Bad relative angle (%f)"), resAngle)
    return res;
}

EClimbTraceResult UBaseMovementComponent::IsEnoughSpace(const FVector& StartLocation,
                                                        const FVector& EndLocation,
                                                        const ECollisionChannel& StartCC, 
                                                        const ECollisionChannel& EndCC)
{
    //check at the start
    if(!IsCharacterCapsuleOk(StartLocation,IsCrouching(),true,StartCC) ||
            !IsCharacterCapsuleOk(StartLocation,IsCrouching(),false,StartCC))
    {
        UE_LOG(LogTemp,Warning, TEXT("IsEnoughSpace error: not enough space for anim start"));
        return EClimbTraceResult::FailAtStart;
    }

    //check for available space at the end of anim (may not be needed)
    if(!EndLocation.IsZero())
    {
        if(!IsCharacterCapsuleOk(EndLocation,IsCrouching(),true,EndCC) ||
                !IsCharacterCapsuleOk(EndLocation,IsCrouching(),false,EndCC))
        {
            UE_LOG(LogTemp,Warning, TEXT("IsEnoughSpace error: not enough space for stand anim end"));
            auto tempEnd = EndLocation + FVector(0,0,-Character->GetDefaultHalfHeight() + 
                            CrouchedHalfHeight);
            if(!IsCharacterCapsuleOk(tempEnd,true,true,EndCC) ||
                    !IsCharacterCapsuleOk(tempEnd,true,false,EndCC))
            {
                UE_LOG(LogTemp,Warning, TEXT("IsEnoughSpace error: not enough space for crouch anim end"));
                return EClimbTraceResult::FailAtEnd;
            }
            else
                return EClimbTraceResult::SuccessCrouch;
        }
    }
    return EClimbTraceResult::SuccessStand;
}

bool UBaseMovementComponent::IsRootOrTeleport() const
{
    return bIsTransitioning || Character->IsPlayingRootMotion();
}

bool UBaseMovementComponent::IsCharacterCapsuleOk(const FVector &CenterLoc, 
                                                    const bool &Crouching, 
                                                    const bool &H2L, 
                                                    const ECollisionChannel& CC) const
{
    //calculate spawn params for capsule shaped sphere trace given center location
    FTraceSpawnParams sParams;
    sParams.Start = sParams.End = CenterLoc;
    if(Crouching)
    {
        sParams.Start.Z += CrouchedHalfHeight - Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
        sParams.End.Z += - CrouchedHalfHeight + Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
    }
    else
    {
        sParams.Start.Z += Character->GetDefaultHalfHeight() - 
            Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
        sParams.End.Z += - Character->GetDefaultHalfHeight() + 
            Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
    }
    if(!H2L)//трейс снизу вверх меняем местами начало и конец
    {
        auto buffer = sParams.Start;
        sParams.Start = sParams.End;
        sParams.End = buffer;
    }
    //do trace check
    FQuat Rotation = Character->GetActorRotation().Quaternion();
    FHitResult HitWall;
    UTraceManager::SphereTraceSingleByChannel(Character->GetWorld(),
                                               Character->GetCapsuleComponent()->GetScaledCapsuleRadius(),
                                               params, sParams, HitWall,
                                               CC, false);
    bool res = HitWall.GetActor() == nullptr;
    if(!res)
        UE_LOG(LogTemp,Warning, TEXT("IsCharacterCapsuleOk error: Not OK!"))
    return res;
}

void UBaseMovementComponent::ClimbDownWall()
{
    if(IsRootOrTeleport())
        return;
    float ClimbDownTracelength = 20;//TODO temp
    FTraceSpawnParams sParams;
    FQuat Rotation(0, 0, 0, 1);
    Rotation = Character->GetActorRotation().Quaternion();
    sParams.Rotation = Rotation;
    auto charBottomHeight = Character->GetActorLocation().Z - Character->GetDefaultHalfHeight();
    if(IsCrouching())
        charBottomHeight = Character->GetActorLocation().Z - CrouchedHalfHeight;
    const auto res = FQuat(FRotator(0, Character->GetActorRotation().Yaw, 0)) * 
                    FQuat(FRotator(0, 180, 0));
    const FRotator charBackDir(res.Rotator());
    sParams.Start = sParams.End = Character->GetActorLocation() + charBackDir.Vector() * 
                                    ClimbDownTracelength;
    sParams.Start.Z = sParams.End.Z = charBottomHeight;
    sParams.End += FVector(0,0,ClimbDownTracelength); 
    const float traceMod = 0.5f;
    const float minHeight = -Character->GetDefaultHalfHeight() * traceMod;
    const float maxHeight = Character->GetDefaultHalfHeight() * traceMod;
    auto hangData = GetClimbStartLoc(sParams, minHeight, maxHeight, false);
    HangOnWall(hangData, true);
}

void UBaseMovementComponent::HangOnFall()
{
    if(IsRootOrTeleport())
        return;
    auto charPelvisLoc = Character->GetActorLocation();
    FTraceSpawnParams sParams;
    FQuat Rotation(0, 0, 0, 1);
    Rotation = Character->GetActorRotation().Quaternion();
    sParams.Rotation = Rotation;
    sParams.Start = charPelvisLoc + FVector(0,0,Character->GetDefaultHalfHeight() -
        Character->GetCapsuleComponent()->GetScaledCapsuleRadius());
    sParams.End = sParams.Start + Character->GetActorRotation().Vector() * LedgeTraceLength;
    const float minHeight = Character->GetDefaultHalfHeight();
    const float maxHeight = Character->GetDefaultHalfHeight() * 2 + 
                        Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
    auto hangData = GetClimbStartLoc(sParams, minHeight, maxHeight);
    HangOnWall(hangData, false);
}

void UBaseMovementComponent::HangOnWall(TSharedPtr<StartClimbData> HangData, bool ClimbDown)
{
    if(HangData.Get() == nullptr)
        return;
    const auto res = FQuat(FRotator(0, HangData.Get()->Rotation.Yaw, 0)) * FQuat(FRotator(0, 180, 0));
    const FRotator wallNormalDir(res.Rotator());
    FVector climbDownStartLocation;
    const auto hangLocation = HangData.Get()->Location + FVector(0, 0, - Character->GetDefaultHalfHeight() + 
                        LocationVertOffset) + 
                        wallNormalDir.Vector() * (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 
                        LocationHorizOffset);// todo заменить на начальную локу
    if(ClimbDown)
        climbDownStartLocation = HangData.Get()->Location + FVector(0, 0, Character->GetDefaultHalfHeight()) + 
                                wallNormalDir.Vector() * (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 
                                LocationHorizOffset);

    //if(GetWalkableFloorAngle() > 90 - HitWallV.ImpactNormal.Rotation().Pitch)
    if(IsEnoughSpace(hangLocation) == EClimbTraceResult::SuccessStand)
    {
        // проверка на наличие места под персом при висении
        FHitResult HitWallAdd;
        FTraceSpawnParams sParams;
        const float traceMod = 0.5f;
        sParams.Start = sParams.End = hangLocation + FVector(0,0,-Character->GetDefaultHalfHeight());
        sParams.End.Z -= Character->GetDefaultHalfHeight() * traceMod;
        UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params, sParams, HitWallAdd,
                                                ECollisionChannel::ECC_WorldStatic, false);
        if(HitWallAdd.GetActor() != nullptr)
        {
            UE_LOG(LogTemp,Warning, TEXT("HangOnWall error: Hit under character"));
            return;
        }
        // проверка на повисание на углу
        if(CheckWallGeometry(*HangData.Get()))
        {
            if(ClimbDown)
                TransitionStart(climbDownStartLocation, wallNormalDir,
                                 ClimbDownMontage, AnimTypes::Parkour, ECustomMovementMode::ClimbingWall);            
            else
                TransitionStart(hangLocation, HangData.Get()->Rotation,
                                 HangMontage, AnimTypes::Parkour, ECustomMovementMode::ClimbingWall);
        }
    }
}

bool UBaseMovementComponent::CheckWallGeometry(const StartClimbData& Data)
{
    FHitResult HitWallLeft;
    FHitResult HitWallRight;
    FTraceSpawnParams sParams;
    // правый трейс
    const auto RightV = (FQuat(FRotator(0, Data.Rotation.Yaw, 0)) * 
                        FQuat(FRotator(0, 90, 0))).Vector();
    sParams.Start = sParams.End = Data.Location + RightV * 
                                    Character->GetCapsuleComponent()->GetScaledCapsuleRadius() +
                                    Data.Rotation.Vector() *
                                    Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
    sParams.Start +=  (FQuat(FRotator(0, Data.Rotation.Yaw, 0)) * 
                    FQuat(FRotator(0, 180, 0))).Vector() * 
                    Character->GetCapsuleComponent()->GetScaledCapsuleRadius() * 2;
    UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params, sParams, HitWallRight,
                                            EWallChannel, false);
    // левый трейс
    const auto LeftV = (FQuat(FRotator(0, Data.Rotation.Yaw, 0)) * 
                        FQuat(FRotator(0, -90, 0))).Vector();
    auto spawnCorrection = LeftV * Character->GetCapsuleComponent()->GetScaledCapsuleRadius() * 2;
    sParams.End += spawnCorrection;
    sParams.Start += spawnCorrection;
    UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params, sParams, HitWallLeft,
                                            EWallChannel, false);
    if(HitWallLeft.GetActor() == nullptr && HitWallRight.GetActor() == nullptr)
    {
        UE_LOG(LogTemp,Warning, TEXT("CheckWallGeometry error: invalid wall geometry"))
        return false;
    }
    return true;
}

void UBaseMovementComponent::ClimbWall()
{
    if(!IsRootOrTeleport())
    {
        const float AnimVertMovement = Character->GetDefaultHalfHeight() * 2;// TODO temp
        const float AnimHorizMovement = Character->GetCapsuleComponent()->GetScaledCapsuleRadius() * 2 +
                                    LocationHorizOffset; // TODO temp
        const FVector AnimEndLocation = Character->GetActorLocation() + FVector(0,0,AnimVertMovement) +
                                    Character->GetActorRotation().Vector() * AnimHorizMovement;//todo temp
        switch(IsEnoughSpace(Character->GetActorLocation(),AnimEndLocation))
        {
            case EClimbTraceResult::SuccessStand:// todo temp just play ClimbWallMontage
            {
                return;
            }
            case EClimbTraceResult::SuccessCrouch: // todo temp just play ClimbWallCrouchedMontage
            {
                //bWantsToCrouch = true;
                return;
            case EClimbTraceResult::FailAtEnd:
            {
                if(bWallJumpAbility)
                {
                    bWasJumpingOff = true;
                    bCanHangAgain = false;
                    ++Character->JumpMaxCount;//для возможности усиления прыжка
                    SetMovementMode(MOVE_Falling);
                    Character->Jump();
                    return;
                }
            }
            default:
                return;
            }
        }
    }
}

void UBaseMovementComponent::GetOffWall()
{
    if(!IsRootOrTeleport())
    {
        SetMovementMode(MOVE_Falling);
        bCanHangAgain = false;
    }
}
// todo всё еще можно повиснуть заново если до это сидел и стена достаточно низкая
bool UBaseMovementComponent::CheckCanHangAgain()
{
    if(!bCanHangAgain)
    {
        if(bWasJumpingOff)
            bCanHangAgain = Character->GetVelocity().Z < 0;
        else if(bWasJustJumping)
        {
            const auto CurrentVelocity = FMath::Abs(Velocity.Z);
            const float JumpVDistanceMod = 0.5f;
            bCanHangAgain = CurrentVelocity < (JumpZVelocity * JumpVDistanceMod);
        }
        else
        {
            const auto NeededVelocity = sqrtf((Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 
                                LocationVertOffset) / 
                                -GetGravityZ() * 2) * 1000;
            const auto CurrentVelocity = FMath::Abs(Velocity.Z);
            bCanHangAgain = CurrentVelocity >= NeededVelocity;
        }
    }
    return bCanHangAgain;
}

void UBaseMovementComponent::JumpOffWall(const EControlDirections& Directions)
{
    // todo сделать чтобы направления были с учетом поворота камеры
    if(!IsRootOrTeleport() && bWallJumpAbility)
	{
        UE_LOG(LogTemp,Warning, TEXT("JumpOffWall: doing jmp off"))
        bCanHangAgain = false;
        bWasJumpingOff = true;
        ++Character->JumpMaxCount;//для возможности усиления прыжка
        SetMovementMode(MOVE_Falling);
        Character->Jump();
        if(Directions == EControlDirections::BackD)
        {
            const auto characterBackDir = (FQuat(FRotator(0, Character->GetActorRotation().Yaw, 0)) * 
                                            FQuat(FRotator(0, 180, 0))).Rotator();
            Character->SetActorRotation(characterBackDir);
            Velocity += characterBackDir.Vector() * JumpWallXYVelocity;
        }
        else if(Directions == EControlDirections::RightD)
            Velocity += Character->GetActorRightVector() * JumpWallXYVelocity;
        else if(Directions == EControlDirections::LeftD)
            Velocity += Character->GetActorRightVector() * -JumpWallXYVelocity;
	}
}

void UBaseMovementComponent::OnMovementModeChanged(EMovementMode prevMode, uint8 prevCustomMode)
{
    if (MovementMode == MOVE_Swimming && IsCrouching())
        UnCrouch();
    if (prevMode == MOVE_Falling)
        bNotifyApex = true;
    if (prevMode == MOVE_Swimming && IsCrouching())
        UnCrouch();
    if (MovementMode == MOVE_Falling || MovementMode == MOVE_Custom)
    {
        fallLocationHeight = GetOwner()->GetActorLocation().Z;
        if(IsCrouching())
        {
            bWantsToCrouch = false;
            bWasCrouching = true;
        }
    }
    if(MovementMode == MOVE_Custom)
    {
        bCanHangAgain = true;
        if(bWasJumpingOff)
        {
            bWasJumpingOff = false;
            --Character->JumpMaxCount;// возврат к стандарт числу прыжков
            if(!bOrientRotationToMovement)
                bOrientRotationToMovement = true;
        }
        if(bWasJustJumping)
            Character->GetAttributesComponent()->Stamina->UnblockRegen();
        bWasJustJumping = false;
    }
    if (MovementMode == MOVE_Walking)
    {
        if(bWasCrouching)
        {
            bWantsToCrouch = true;
            bWasCrouching = false;
        }
        bCanHangAgain = true;
        if(bWasJumpingOff)
        {
            bWasJumpingOff = false;
            --Character->JumpMaxCount;
            if(!bOrientRotationToMovement)
                bOrientRotationToMovement = true;
        }
        if(bWasJustJumping)
            Character->GetAttributesComponent()->Stamina->UnblockRegen();
        bWasJustJumping = false;
    }
    if (prevMode == MOVE_Falling && MovementMode == MOVE_Walking)
    {

        if(!bFallDamageInvul)
            FallDamageCheck();
    }
    if (prevMode == MOVE_Walking)
    {
        if(bIsSprinting)
        {
            if(GetWorld()->GetTimerManager().IsTimerActive(SprintConsumeTimer))
            GetWorld()->GetTimerManager().ClearTimer(SprintConsumeTimer);
            Character->GetAttributesComponent()->Stamina->UnblockRegen();
            bIsSprinting = false;
        }
    }
    Super::OnMovementModeChanged(prevMode, prevCustomMode);
}

float UBaseMovementComponent::GetMaxSpeed() const
{
    switch (MovementMode)
    {
    case MOVE_Walking:
    case MOVE_NavWalking:
    {
        if (IsCrouching() && bIsRunning && WeightFactor <= 1)
            return MaxRunSpeedCrouched;
        else if (IsCrouching() && !bIsRunning)
            return MaxWalkSpeedCrouched;
        else if (!bWantToSprint && !bIsRunning)
            return MaxWalkSpeed;
        else if (!bWantToSprint && bIsRunning && WeightFactor <= 1)
            return MaxRunSpeed;
        else if (bWantToSprint && Character->GetAttributesComponent()->Stamina->GetValue() > 0 && 
                    WeightFactor <= 1)
            return MaxSprintSpeed;
        else if(WeightFactor <= 1)
            return MaxRunSpeed;
        else
            return MaxWalkSpeed;

    }
    case MOVE_Falling:
        return MaxWalkSpeed;
    case MOVE_Swimming:
        return MaxSwimSpeed;
    case MOVE_Flying:
        return MaxFlySpeed;
    case MOVE_Custom:
        return MaxCustomMovementSpeed;
    case MOVE_None:
    default:
        return 0.f;
    }
}

void UBaseMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
    Super::PhysWalking(deltaTime, Iterations);

    if(bWantToSprint && Velocity.Size() > MaxWalkSpeedCrouched - 1.f &&
        !IsCrouching() && Character->GetAttributesComponent()->Stamina->GetValue() > 0
        && WeightFactor <= 1)
    {
        if(!bIsSprinting)
        {
            bIsRunning = true;
            if(!GetWorld()->GetTimerManager().IsTimerActive(SprintConsumeTimer))
                GetWorld()->GetTimerManager().SetTimer(SprintConsumeTimer, this, &UBaseMovementComponent::OnSprintConsume, 
                                                SprintConsumeTime, true);
            Character->GetAttributesComponent()->Stamina->BlockRegen();
            bIsSprinting = true;
        }
    }
    else
    {
        if(bIsSprinting)
        {
            if(GetWorld()->GetTimerManager().IsTimerActive(SprintConsumeTimer))
                GetWorld()->GetTimerManager().ClearTimer(SprintConsumeTimer);
            Character->GetAttributesComponent()->Stamina->UnblockRegen();
            bIsSprinting = false;
        }
    }
    //UE_LOG(LogTemp, Warning, TEXT("velocity %f"), Velocity.Size());
    //UE_LOG(LogTemp, Warning, TEXT("IsSprint %s"), bIsSprinting ? TEXT("true") : TEXT("false"));
}

void UBaseMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
    Super::PhysFalling(deltaTime, Iterations);
}

void UBaseMovementComponent::PhysCustom(float deltaTime, int32 Iterations)///TODO разобраться в стандартной реализации
{
    //UE_LOG(LogTemp,Warning, TEXT("root %s"),Character->IsPlayingRootMotion() ? TEXT("true" : TEXT("false")));
    if (deltaTime < MIN_TICK_TIME)
    {
        return;
    }

    //RestorePreAdditiveRootMotionVelocity();

    if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
    {
        //if( bCheatFlying && Acceleration.IsZero() )
        //{
            Velocity = FVector::ZeroVector;
        //}
        const float Friction = 0.5f * GetPhysicsVolume()->FluidFriction;
        CalcVelocity(deltaTime, Friction, true, GetMaxBrakingDeceleration());
    }

    //ApplyRootMotionToVelocity(deltaTime);

    Iterations++;
    bJustTeleported = false;

    FVector OldLocation = UpdatedComponent->GetComponentLocation();
    const FVector Adjusted = Velocity * deltaTime;
    FHitResult Hit(1.f);
    SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

    if (Hit.Time < 1.f)
    {
        const FVector GravDir = FVector(0.f, 0.f, -1.f);
        const FVector VelDir = Velocity.GetSafeNormal();
        const float UpDown = GravDir | VelDir;

        bool bSteppedUp = false;
        if ((FMath::Abs(Hit.ImpactNormal.Z) < 0.2f) && (UpDown < 0.5f) && (UpDown > -0.2f) && CanStepUp(Hit))
        {
            float stepZ = UpdatedComponent->GetComponentLocation().Z;
            bSteppedUp = StepUp(GravDir, Adjusted * (1.f - Hit.Time), Hit);
            if (bSteppedUp)
            {
                OldLocation.Z = UpdatedComponent->GetComponentLocation().Z + (OldLocation.Z - stepZ);
            }
        }

        if (!bSteppedUp)
        {
            //adjust and try again
            HandleImpact(Hit, deltaTime, Adjusted);
            SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
        }

    }


    if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
    {
        Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
    }

    //TODO for parkur
    switch(CustomMovementMode)
    {
        //case ECustomMovementMode::ClimbingLadder: // deprecated
        case ECustomMovementMode::ClimbingWall:
        case ECustomMovementMode::ClimbingLedge:
        //caseECustomMovementMode::Slide: // deprecated
        case ECustomMovementMode::Dodge:
        default:
            return;
    }
}

//проверка получит ли персонаж урон от падения и сколько
//TODO добавить реализацию получения урона
bool UBaseMovementComponent::FallDamageCheck()
{
    const float difference = fallLocationHeight - Character->GetActorLocation().Z;
    fallLocationHeight = 0;
    if (MinFallHeightThreshold < difference)
    {
        ApplyFallDamage(difference);
        return true;
    }
    else
    return false;
}

void UBaseMovementComponent::ApplyFallDamage(float FallDistance)
{
    float damage = Character->GetAttributesComponent()->Health->GetCurrentMaxValue();
    if (FallDistance < MaxfallHeightThreshold)
        damage *= (FallDistance - MinFallHeightThreshold) / MinFallHeightThreshold;	// NOT NULL!
}

void UBaseMovementComponent::TransitionStart(const FVector& EndLoc,
                    const FRotator& EndRot,
                    UAnimMontage* AnimToPlay,
                    AnimTypes Type, 
                    ECustomMovementMode InMode)
{
    if(bIsTransitioning){
        UE_LOG(LogTemp, Warning, TEXT("TransitionStart error: already transitioning"))
        return;
    }
    if(AnimToPlay == nullptr || !AnimToPlay->IsValidSectionName(FTransitionSectionNames::End)){
        UE_LOG(LogTemp, Warning, TEXT("TransitionStart error: bad animation to play"))// todo add return after anims addition
            return;
    }
    bIsTransitioning = true;
    // if none then return to normal movement when finished
    if(InMode == ECustomMovementMode::NoParkour)
        TransitionFinishMode = MovementMode;
    else
        TransitionFinishMode = EMovementMode::MOVE_Custom;
    SetMovementMode(EMovementMode::MOVE_Custom, InMode);
    TransitionLocEnd = MakeUnique<FVector>(EndLoc);
    TransitionRotEnd = MakeUnique<FRotator>(EndRot);
    CurrentTransitionAnim = AnimToPlay;
    if(AnimToPlay->IsValidSectionName(FTransitionSectionNames::Start))
    {
        Character->StartAnimation(CurrentTransitionAnim, Type, AnimPlayRate,
            FTransitionSectionNames::Start);
        animType = Type;
    }
    else
        UE_LOG(LogTemp, Warning, TEXT("TransitionStart info: no anim start section"))
}

void UBaseMovementComponent::On_TransitionFinished()
{
    check(CurrentTransitionAnim != nullptr && "Transition: no anim!")
        Character->StartAnimation(CurrentTransitionAnim, animType, AnimPlayRate,
            FTransitionSectionNames::End);
    TransitionEnd();
    UE_LOG(LogTemp, Warning, TEXT("TransitionFinished!"))
}

void UBaseMovementComponent::TransitionProcess(float deltaTime)
{
    // todo add cancel if can't reach the end
    if(TransitionLocEnd.Get()->Equals(Character->GetActorLocation(),1) && 
        TransitionRotEnd.Get()->Equals(Character->GetActorRotation(),1))
        On_TransitionFinished();
    else
    {
        const bool locSweep = Character->SetActorLocation(FMath::Lerp(Character->GetActorLocation(), 
                *TransitionLocEnd.Get(), TransitionMoveSpeed * deltaTime), true);
        const bool rotSweep = Character->SetActorRotation(FMath::Lerp(Character->GetActorRotation(),
                *TransitionRotEnd.Get(), TransitionRotationSpeed * deltaTime));
        if(!locSweep || !rotSweep)
        {
            TransitionEnd();
            UE_LOG(LogTemp, Warning, TEXT("TransitionProcess error: collision hit!!")) 
        }
    }
}

void UBaseMovementComponent::On_TransitionInterrupt()
{
    TransitionEnd();
    UE_LOG(LogTemp, Warning, TEXT("TransitionInterrupt error: interrupted!!"))
}

void UBaseMovementComponent::TransitionEnd()
{
    check(bIsTransitioning && "Transition: can't' cancel")
    bIsTransitioning = false;
    if(MovementMode != EMovementMode::MOVE_Custom)
        SetMovementMode(TransitionFinishMode);
    Character->StopAnimMontage(CurrentTransitionAnim);
    CurrentTransitionAnim = nullptr;
    TransitionLocEnd.Release();
    TransitionRotEnd.Release();
}

void UBaseMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    //для игрока каст в игрового перса
    Character = Cast<ACharacterBase>(PawnOwner);
    check(Character != nullptr && "UBaseMovementComponent error: No pawn")
    params.ActorsToIgnore.Add(CharacterOwner);
    MoveAbilitiesCurrent = MoveAbilitiesDefault;
}

void UBaseMovementComponent::NotifyJumpApex()
{
    /*
    // todo действие при достижении вершины прыжка
    UE_LOG(LogTemp, Warning, TEXT("Movement component Test"));
    FVector start = GetOwner()->GetActorLocation();
    FVector end = start + FVector(0, 1000, -5000);
    DrawDebugLine(GetOwner()->GetWorld(),start,end,FColor::Red,false,4.f);
    */
}

void UBaseMovementComponent::TickComponent(float DeltaTime, 
                                enum ELevelTick TickType,
                                FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    VisualizeMovement();// debug
    if(bIsTransitioning)
        TransitionProcess(DeltaTime);
    //UE_LOG(LogTemp, Warning, TEXT("iswalkablefloor %s"), CurrentFloor.IsWalkableFloor() ? TEXT("true") : TEXT("false"));
    //UE_LOG(LogTemp, Warning, TEXT("floor distance %f"), CurrentFloor.GetDistanceToFloor());
}

void UBaseMovementComponent::OnSprintConsume()
{
    if(Character->GetAttributesComponent()->Stamina->Consume(StaminaCostValues.Sprint))
        GetWorld()->GetTimerManager().SetTimer(SprintConsumeTimer, this, &UBaseMovementComponent::OnSprintConsume, 
											SprintConsumeTime, true);
}

void UBaseMovementComponent::Update()
{
    //const auto dex = UAttributesHelper::GetAttribute(Character->GetAttributesComponent()->CurrentAttributes, EAttributes::Dex);
	const auto tumble = USkillHelper::GetValue(Character->GetAttributesComponent()->CurrentSkills, ESkill::Tumble);
    const auto dexMod = UAttributesHelper::GetModifier(
    Character->GetAttributesComponent()->CurrentAttributes,EAttributes::Dex);
    const auto tumbleMod = UAttributesHelper::GetModifier(Character->GetAttributesComponent()->CurrentAttributes, 
        FSkillList::ModifierMap[(int)ESkill::Tumble]); // более verbose способ получения модификатора
    
    UE_LOG(LogTemp, Warning, TEXT("skill val %d"), USkillHelper::GetValue(Character->GetAttributesComponent()->CurrentSkills, ESkill::Tumble))
    UE_LOG(LogTemp, Warning, TEXT("skill mod %d"), UAttributesHelper::GetModifier(Character->GetAttributesComponent()->CurrentAttributes, 
        FSkillList::ModifierMap[(int)ESkill::Tumble]))
    // ловкость

    MinFallHeightThreshold = GetSigmoidValue(dexMod, abilityMod, MinFallHeightThresholdBase * (1.f - FallDamageScale), 
        MinFallHeightThresholdBase * (1.f + FallDamageScale));
    MaxfallHeightThreshold = GetSigmoidValue(dexMod, abilityMod, MaxfallHeightThresholdBase * (1.f - FallDamageScale), 
        MaxfallHeightThresholdBase * (1.f + FallDamageScale));

    JumpWallZVelocity = GetSigmoidValue(dexMod, abilityMod, JumpWallZVelocityBase * (1.f - SpeedScale), JumpWallZVelocityBase * (1.f + SpeedScale));
    JumpWallXYVelocity = GetSigmoidValue(dexMod, abilityMod, JumpWallXYVelocityBase * (1.f - SpeedScale), JumpWallXYVelocityBase * (1.f + SpeedScale));
    JumpZVelocity = GetSigmoidValue(dexMod, abilityMod, JumpZVelocityBase * (1.f - SpeedScale), JumpZVelocityBase * (1.f + SpeedScale));
    AnimPlayRate = GetSigmoidValue(dexMod, abilityMod, AnimPlayRateBase * (1.f - AnimPlayRateScale), AnimPlayRateBase * (1.f + AnimPlayRateScale));
    MaxOverweightMoveFactor = GetSigmoidValue(dexMod, abilityMod, 1.f, MaxOverweightMoveFactorBase * 1.2f); // temp

    // акробатика
    
    AirControl = GetSigmoidValue(tumble + tumbleMod, skillMod, -1.f + AirControlBase, 1.f);
    if(AirControl < 0)
        AirControl = 0.f;

    // не нужно, не работает, не будет работать т.к. в воздухе нет ускорения, получается постоянное замедление
    //BrakingDecelerationFalling = GetSigmoidValue(tumble + tumbleMod, skillMod, -BrakingDecelerationFallingBase, BrakingDecelerationFallingBase);
    //if(BrakingDecelerationFalling < 0)
    //    BrakingDecelerationFalling = 0.f;
    //FallingLateralFriction = GetSigmoidValue(tumble, skillMod, FallingLateralFrictionBase * (1.f - FrictionScale), FallingLateralFrictionBase * (1.f + FrictionScale));

    Character->JumpMaxHoldTime = GetSigmoidValue(tumble + tumbleMod, skillMod, -JumpMaxHoldTimeBase, JumpMaxHoldTimeBase); // ue4 имплементация (поэтому в персе)
    if(Character->JumpMaxHoldTime < 0)
        Character->JumpMaxHoldTime = 0.f;
    
    // нагрузка
    // todo разделить на категории загруженности как в дарк соулс
    WeightFactor = GetWeightFactor();
    MaxAcceleration = GetSigmoidValue(dexMod + weightFactorBalanceMod * (-WeightFactor), weightMod, MaxAccelerationBase * (1.f - AccelerationScale), 
        MaxAccelerationBase * (1.f + AccelerationScale));
    BrakingDecelerationWalking = GetSigmoidValue(dexMod + weightFactorBalanceMod * (-WeightFactor), weightMod, 
        BrakingDecelerationWalkingBase * (1.f - AccelerationScale), BrakingDecelerationWalkingBase * (1.f + AccelerationScale));
    BrakingFriction = GetSigmoidValue(dexMod + weightFactorBalanceMod * (-WeightFactor), weightMod, BrakingFrictionBase * (1.f - FrictionScale), BrakingFrictionBase * (1.f + FrictionScale));

	//TODO: рассчитать реальный мод скорости с учетом текущих бафов
    //Max Speed update
    const float speedMod = GetSigmoidValue(dexMod + weightFactorBalanceMod * (-WeightFactor), weightMod, 1.f - SpeedScale, 1.f + SpeedScale); // нормально
    CalculateMaxSpeeds(speedMod);

    // debug log todo
    UE_LOG(LogTemp, Warning, TEXT("MinFallHeightThreshold %f"), MinFallHeightThreshold)
    UE_LOG(LogTemp, Warning, TEXT("MaxfallHeightThreshold %f"), MaxfallHeightThreshold)
    UE_LOG(LogTemp, Warning, TEXT("JumpWallZVelocity %f"), JumpWallZVelocity)
    UE_LOG(LogTemp, Warning, TEXT("JumpWallXYVelocity %f"), JumpWallXYVelocity)
    UE_LOG(LogTemp, Warning, TEXT("JumpZVelocity %f"), JumpZVelocity)
    UE_LOG(LogTemp, Warning, TEXT("AnimPlayRate %f"), AnimPlayRate)
    UE_LOG(LogTemp, Warning, TEXT("AirControl %f"), AirControl)
    UE_LOG(LogTemp, Warning, TEXT("BrakingDecelerationFalling %f"), BrakingDecelerationFalling)
    UE_LOG(LogTemp, Warning, TEXT("FallingLateralFriction %f"), FallingLateralFriction)
    UE_LOG(LogTemp, Warning, TEXT("JumpMaxHoldTime %f"), Character->JumpMaxHoldTime)
    UE_LOG(LogTemp, Warning, TEXT("MaxAcceleration %f"), MaxAcceleration)
    UE_LOG(LogTemp, Warning, TEXT("BrakingDecelerationWalking %f"), BrakingDecelerationWalking)
    UE_LOG(LogTemp, Warning, TEXT("BrakingFriction %f"), BrakingFriction)
    
    UnSetUpdate();
}

void UBaseMovementComponent::Crouch(bool bClientSimulation)
{
    if((MoveAbilitiesCurrent & EMoveAbilities::CanCrouch) == EMoveAbilities::CanCrouch)
    {
        Super::Crouch(bClientSimulation);
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("blocked crouch"));
    bWantsToCrouch = false;
}

bool UBaseMovementComponent::DoJump(bool bReplayingMoves)
{
    if((MoveAbilitiesCurrent & EMoveAbilities::CanJump) != EMoveAbilities::CanJump || 
        bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) == 1.f ||
        WeightFactor > 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("blocked jump"));
        return false;
    }
    if(Character->GetAttributesComponent()->Stamina->GetValue() <= 0)
        return false;
    if(!Character->bWasJumping)
        Character->GetAttributesComponent()->Stamina->Consume(StaminaCostValues.Jump);
    if(MovementMode != EMovementMode::MOVE_Falling)
        Character->GetAttributesComponent()->Stamina->BlockRegen();
    if(bWasJumpingOff)
    {
        if(bOrientRotationToMovement)
            bOrientRotationToMovement = false;
        Velocity.Z = FMath::Max(Velocity.Z, JumpWallZVelocity);
        return true;
    }
    else
        return Super::DoJump(bReplayingMoves);

}

bool UBaseMovementComponent::CanMove() const
{
    if(WeightFactor > MaxOverweightMoveFactor)
        return false;
    return true;
}

float UBaseMovementComponent::GetWeightFactor() const
{
    const float maxWeight = Character->MaxWeight;
    const float epsilon = 0.001f;
    const float invMod = 0.5f;
    const float eqMod = 1.f;
    if(maxWeight < epsilon)
        return 0;
    return (Character->GetInventoryComponent()->CurrentWeight * invMod + 
        Character->GetEquipmentComponent()->CurrentWeight * eqMod) / maxWeight;
}

void UBaseMovementComponent::ParkourInterrupt(UAnimMontage* animMontage, bool bInterrupted)
{
    if(bInterrupted)
        UE_LOG(LogTemp, Warning, TEXT("ParkourInterrupt"))
    else
        UE_LOG(LogTemp, Warning, TEXT("ParkourFinishedNormally"))
    // todo reset movement mode to some valid state and restore character capsule size if modified
}

void UBaseMovementComponent::DodgeInterrupt(UAnimMontage* animMontage, bool bInterrupted)
{
    if(bInterrupted)
        UE_LOG(LogTemp, Warning, TEXT("DodgeInterrupt"))
    else
        UE_LOG(LogTemp, Warning, TEXT("DodgeFinishedNormally"))
    // todo disable invul
}

void UBaseMovementComponent::On_StartAnimation()
{
    Character->StartAnimation(VaultMontageHigh, AnimTypes::Parkour, 1);
    auto animInstance2 = Cast<UCharacterAnimInstance>(Character->GetMesh()->GetAnimInstance());
	auto montageInstance2 = animInstance2->GetActiveInstanceForMontage(VaultMontageHigh);
	montageInstance2->OnMontageBlendingOutStarted.BindUObject(this, &UBaseMovementComponent::ParkourInterrupt);
}

void UBaseMovementComponent::On_InterruptAnimation()
{
    Character->InterruptAction(ECCState::NoCC);
}
