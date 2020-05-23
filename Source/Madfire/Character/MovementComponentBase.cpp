// Fill out your copyright notice in the Description page of Project Settings.

#include "MovementComponentBase.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "CharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PhysicsVolume.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"

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
    LocationHorizOffset = 5;
    LocationVertOffset = 5;
    MoveAbilitiesCurrent = MoveAbilitiesDefault = 0xff;
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

FTraceSpawnParams UBaseMovementComponent::GetHorizTraceSpawnParams(bool IsLow)
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
    UE_LOG(LogTemp,Warning, TEXT("CanVault or climb %s"), IsLow ? TEXT("low") : TEXT("high"));
    const FRotator CharRotation = Character->GetActorRotation();
    //setting spawn parameters
    FTraceSpawnParams sParamsH = GetHorizTraceSpawnParams(IsLow);
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
// todo исправить проверку на волт и его трейсы параметры спауна
// todo при проверке на наличие места берется текущий размер капсулы
bool UBaseMovementComponent::DoVault(const StartClimbData& locationData, 
                                    bool IsLow)
{
    ///TODO add stamina calculation and usage
    // vector in the direction opposite to the wall's normal
    const auto res = FQuat(FRotator(0, locationData.Rotation.Yaw, 0)) * FQuat(FRotator(0, 180, 0));
    const FRotator wallNormalDir(res.Rotator());
    //check for available space at the position before anim
    float charPelvisheight = Character->GetActorLocation().Z;;
    if(IsCrouching())
        charPelvisheight = Character->GetActorLocation().Z + 
            Character->GetDefaultHalfHeight() - CrouchedHalfHeight;
    // коррекция по высоте
    FVector VaultAnimationStartLocation = locationData.Location +
                                        FVector::UpVector * LocationVertOffset +
                                        wallNormalDir.Vector() *
                                        (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 
                                        LocationHorizOffset);
    if(locationData.Location.Z < charPelvisheight)
        VaultAnimationStartLocation.Z = charPelvisheight +
                                        Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 
                                        LocationVertOffset;
    const FVector VaultAnimationEndLocation = VaultAnimationStartLocation +
                                             FRotator(0, locationData.Rotation.Yaw, 0).Vector() *
                                            (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() * 2 + 
                                            LocationHorizOffset + MaxVaultLedgeSize);// todo temp

    if(IsEnoughSpace(VaultAnimationStartLocation) == EClimbTraceResult::SuccessStand)
    {
        SetMovementMode(EMovementMode::MOVE_Custom,ECustomMovementMode::ClimbingLedge);
         ///TODO replace with smoothness
        Character->SetActorLocation(VaultAnimationStartLocation, false, nullptr, ETeleportType::None);
        Character->SetActorRotation(locationData.Rotation, ETeleportType::None);
        if(IsLow && VaultMontageLow != nullptr)
         ///TODO calculate playRate with agility and when done set movement mode
         //todo сделать на время выполнения анимации капсуле размер капсулы в присяде
            Character->PlayAnimMontage(VaultMontageLow,1);
        else if(!IsLow && VaultMontageHigh != nullptr)
            Character->PlayAnimMontage(VaultMontageHigh,1);
        else
        {
            Character->SetActorLocation(VaultAnimationEndLocation);
            SetMovementMode(EMovementMode::MOVE_Walking);
        }
        return true;

    }
    return false;
}

bool UBaseMovementComponent::DoClimbLedge(const StartClimbData& locationData, bool IsLow)
{
    ///TODO add stamina calculation and usage
    // vector in the direction opposite to the wall's normal
    const auto res = FQuat(FRotator(0, locationData.Rotation.Yaw, 0)) * FQuat(FRotator(0, 180, 0));
    const FRotator wallNormalDir(res.Rotator());
    // check for available space at the position before anim
    auto ClimbAnimHorizDistance = 50;///TODO temp длина пути анимации
    float charPelvisheight = Character->GetActorLocation().Z;
    if(IsCrouching())
        charPelvisheight = Character->GetActorLocation().Z + Character->GetDefaultHalfHeight() - 
                            CrouchedHalfHeight;
    // коррекция по высоте
    FVector ClimbledgeAnimationStartLocation = locationData.Location +
                                        FVector::UpVector * LocationVertOffset
                                        + wallNormalDir.Vector() *
                                        (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 
                                        LocationHorizOffset);
    if(locationData.Location.Z < charPelvisheight)
        ClimbledgeAnimationStartLocation.Z = charPelvisheight +
                                        Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 
                                        LocationVertOffset;
    const FVector ClimbledgeAnimationEndLocation = ClimbledgeAnimationStartLocation +
            FVector(0,0,Character->GetDefaultHalfHeight()) +
            locationData.Rotation.Vector() *
            (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + LocationHorizOffset + 
            ClimbAnimHorizDistance);// todo temp
    // check for available space at the start position before anim
    switch(IsEnoughSpace(ClimbledgeAnimationStartLocation, ClimbledgeAnimationEndLocation))
    {
        case EClimbTraceResult::SuccessStand:
        {
            SetMovementMode(EMovementMode::MOVE_Custom,ECustomMovementMode::ClimbingLedge);
             //TODO replace with smooth motion
            Character->SetActorLocation(ClimbledgeAnimationStartLocation, false, nullptr, ETeleportType::None);
            Character->SetActorRotation(locationData.Rotation, ETeleportType::None);
            if(IsLow && ClimbLedgeMontageLow != nullptr)
             //TODO calculate playRate with agility and when done set movement mode
                Character->PlayAnimMontage(ClimbLedgeMontageLow,1);
            else if(!IsLow && ClimbLedgeMontageHigh != nullptr)
                Character->PlayAnimMontage(ClimbLedgeMontageHigh,1);
            else
            {
                Character->SetActorLocation(ClimbledgeAnimationEndLocation);
                SetMovementMode(EMovementMode::MOVE_Walking);
            }
            return true;
        }
        case EClimbTraceResult::SuccessCrouch:
        {
            SetMovementMode(EMovementMode::MOVE_Custom,ECustomMovementMode::ClimbingLedge);
             //TODO replace with smooth motion
            Character->SetActorLocation(ClimbledgeAnimationStartLocation, false, nullptr, ETeleportType::None);
            Character->SetActorRotation(locationData.Rotation, ETeleportType::None);
            if(!IsCrouching())
            {
                Character->GetCapsuleComponent()->SetCapsuleHalfHeight(CrouchedHalfHeight);
                bWantsToCrouch = true;
            }
            //TODO calculate playRate with agility and when done set movement mode
            if(IsLow && ClimbLedgeCrouchedMontageLow != nullptr)
                Character->PlayAnimMontage(ClimbLedgeCrouchedMontageLow,1); 
            else if(!IsLow && ClimbLedgeCrouchedMontageHigh != nullptr)
                Character->PlayAnimMontage(ClimbLedgeCrouchedMontageHigh,1);
            else
            {
                Character->SetActorLocation(ClimbledgeAnimationEndLocation);
                SetMovementMode(EMovementMode::MOVE_Walking);
            }
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
                                               EWallChannel,
                                               false);
    if(vaultHit.GetActor() == nullptr)// проверка достаточного места за преграждением для волта
    {
        sParams.Rotation = Rotation;
        sParams.Start = sParams.End;
               sParams.End.Z -= Character->GetDefaultHalfHeight()/2;
        UTraceManager::BoxTraceSingleByChannel(Character->GetWorld(),FVector::UpVector *
                                               Character->GetDefaultHalfHeight()/4,params,
                                               sParams,vaultHit,EWallChannel,false);
        if(vaultHit.GetActor() == nullptr)
        {
            return true;
        }
    }
    UE_LOG(LogTemp,Warning, TEXT("VaultSpaceCheck: fail"))
    return false;
}

TSharedPtr<StartClimbData> UBaseMovementComponent::GetClimbStartLoc(const FTraceSpawnParams& startTrace,
                                                const float& minHeight,
                                                const float& maxHeight,
                                                bool checkAngle)
{
    auto CharRotation = Character->GetActorRotation();
    auto FinalLocation = MakeShared<StartClimbData>();
    auto charBottomHeight = Character->GetActorLocation().Z - Character->GetDefaultHalfHeight();
    if(IsCrouching())
        charBottomHeight = Character->GetActorLocation().Z - CrouchedHalfHeight;
    FHitResult HitWallH;
    FTraceSpawnParams sParamsH = startTrace;
    auto traceCapsuleHalfHeight = (maxHeight - minHeight) / 2;
    int NumOfIterations = 1;
    auto currentTraceHeight = sParamsH.Start.Z;
    // проверка трейсами на подходящий край для зацепа за стену
    for(;minHeight <= currentTraceHeight <= maxHeight; ++NumOfIterations)
    {
        UTraceManager::CapsuleTraceSingleByChannel(Character->GetWorld(), 
                                                   Character->GetCapsuleComponent()->GetScaledCapsuleRadius(),
                                                   traceCapsuleHalfHeight, params, sParamsH, HitWallH,
                                                   EWallChannel, false);
        // нашел ли стену
        if(HitWallH.GetActor() == nullptr)
        {
            UE_LOG(LogTemp,Warning, TEXT("GetClimbStartLoc: trace miss"))
            return nullptr;
        }
        // подходящий ли угол между персом и стеной
        if(checkAngle && !checkRelativeRotation(HitWallH.ImpactNormal.Rotation().Yaw))
            return nullptr;
        // ограничение по числу итераций
        if(NumOfIterations >= 10)// temp
        {
            UE_LOG(LogTemp,Warning, TEXT("GetClimbStartLoc: exceeded iter limit"))
            return nullptr;
        }
        // проверим есть ли место для капсулы перса для начала анимации
        const FRotator wallNormalDir(0,HitWallH.ImpactNormal.Rotation().Yaw,0);
        FTraceSpawnParams sParamsSPace;
        sParamsSPace.Start = sParamsSPace.End = HitWallH.ImpactPoint + wallNormalDir.Vector() * 
                            LocationHorizOffset;
        sParamsSPace.Start.Z = charBottomHeight + maxHeight;
        sParamsSPace.End.Z = charBottomHeight + minHeight + LocationVertOffset;
        FHitResult HitSpace;
        UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params,
                                            sParamsSPace, HitSpace,
                                            EWallChannel, false);
        if(HitSpace.GetActor() == nullptr)
            break;
        else
            UE_LOG(LogTemp,Warning, TEXT("GetClimbStartLoc: not enough space"))
        // обновление параметров трейса для новой точки
        currentTraceHeight = HitWallH.ImpactPoint.Z - charBottomHeight;
        if(fabsf(currentTraceHeight-maxHeight) >= fabsf(currentTraceHeight-minHeight))
            traceCapsuleHalfHeight = fabsf(currentTraceHeight-minHeight);
         else
            traceCapsuleHalfHeight = fabsf(currentTraceHeight-maxHeight);
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
    sParamsV.Start.Z = charBottomHeight + maxHeight + heightAllowance;
    sParamsV.End.Z = charBottomHeight + minHeight;
    FHitResult HitWallV;
    UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params,
                                            sParamsV, HitWallV,
                                            EWallChannel, false);
    if(HitWallV.GetActor() == nullptr || minHeight > (HitWallV.ImpactPoint.Z - charBottomHeight)
    || (HitWallV.ImpactPoint.Z - charBottomHeight) > maxHeight)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetClimbStartLoc: vert trace miss"))
        return nullptr;
    }
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
    auto resAngle = fabsf(acos(FVector::DotProduct(charsV, wallsV)) * 180.f / PI - 180.f);
    const bool res = resAngle < MaxClimbRotationAngle;
    if(!res)
        UE_LOG(LogTemp,Warning, TEXT("Bad relative angle (%f)"), resAngle)
    return res;
}

EClimbTraceResult UBaseMovementComponent::IsEnoughSpace(const FVector& StartLocation,
                                                        const FVector& EndLocation,
                                                        const ECollisionChannel& startCC, 
                                                        const ECollisionChannel& endCC)
{
    //check at the start
    if(!IsCharacterCapsuleOk(StartLocation,IsCrouching(),true,startCC) ||
            !IsCharacterCapsuleOk(StartLocation,IsCrouching(),false,startCC))
    {
        UE_LOG(LogTemp,Warning, TEXT("not enough space for anim start"));
        return EClimbTraceResult::FailAtStart;
    }

    //check for available space at the end of anim (may not be needed)
    if(!EndLocation.IsZero())
    {
        if(!IsCharacterCapsuleOk(EndLocation,IsCrouching(),true,endCC) ||
                !IsCharacterCapsuleOk(EndLocation,IsCrouching(),false,endCC))
        {
            UE_LOG(LogTemp,Warning, TEXT("not enough space for stand anim end"));
            auto tempEnd = EndLocation + FVector(0,0,-Character->GetDefaultHalfHeight() + 
                            CrouchedHalfHeight);
            if(!IsCharacterCapsuleOk(tempEnd,true,true,endCC) ||
                    !IsCharacterCapsuleOk(tempEnd,true,false,endCC))
            {
                UE_LOG(LogTemp,Warning, TEXT("not enough space for crouch anim end"));
                return EClimbTraceResult::FailAtEnd;
            }
            else
                return EClimbTraceResult::SuccessCrouch;
        }
    }
    return EClimbTraceResult::SuccessStand;
}

bool UBaseMovementComponent::IsRootOrTeleport()
{
    if(bIsTeleporting || Character->IsPlayingRootMotion())
        return true;
    return false;
}

bool UBaseMovementComponent::IsCharacterCapsuleOk(const FVector &centerLoc, 
                                                    const bool &crouching, 
                                                    const bool &H2L, 
                                                    const ECollisionChannel& CC) const
{
    //calculate spawn params for capsule shaped sphere trace given center location
    FTraceSpawnParams sParams;
    sParams.Start = sParams.End = centerLoc;
    if(crouching)
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
        UE_LOG(LogTemp,Warning, TEXT("IsCharacterCapsuleOk: Not OK!"))
    return res;
}

void UBaseMovementComponent::ClimbDownWall()
{
    if(IsRootOrTeleport())
        return;
    float ClimbDownTracelenght = 20;//TODO temp
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
                                    ClimbDownTracelenght;
    sParams.Start.Z = sParams.End.Z = charBottomHeight;
    sParams.End += FVector(0,0,ClimbDownTracelenght); 
    const float minHeight = -Character->GetDefaultHalfHeight() / 2;
    const float maxHeight = Character->GetDefaultHalfHeight() / 2;
    auto hangData = GetClimbStartLoc(sParams, minHeight, maxHeight, false);
    HangOnWall(hangData);
}

void UBaseMovementComponent::HangOnFall()
{
    if(IsRootOrTeleport())
        return;
    auto charPelvisLoc = Character->GetActorLocation();
    if(IsCrouching())
        charPelvisLoc = Character->GetActorLocation() + FVector(0, 0, - CrouchedHalfHeight);
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
    HangOnWall(hangData);
}
//todo сделать проверку чтоб нельзя было повиснуть на углу
void UBaseMovementComponent::HangOnWall(TSharedPtr<StartClimbData> hangData)
{
    if(hangData.Get() == nullptr)
        return;
    const auto res = FQuat(FRotator(0, hangData.Get()->Rotation.Yaw, 0)) * FQuat(FRotator(0, 180, 0));
    const FRotator wallNormalDir(res.Rotator());
    auto hangLocation = hangData.Get()->Location + FVector(0, 0, - Character->GetDefaultHalfHeight() + 
                        LocationVertOffset) + 
                        wallNormalDir.Vector() * (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 
                        LocationHorizOffset);
    //if(GetWalkableFloorAngle() > 90 - HitWallV.ImpactNormal.Rotation().Pitch)
    if(IsEnoughSpace(hangLocation) == EClimbTraceResult::SuccessStand)
    {
        // проверка на наличие места под персом при висении
        FHitResult HitWallAdd;
        FTraceSpawnParams sParams;
        FQuat Rotation(0, 0, 0, 1);
        Rotation = Character->GetActorRotation().Quaternion();
        sParams.Rotation = Rotation;
        sParams.Start = sParams.End = hangLocation + FVector(0,0,-Character->GetDefaultHalfHeight());
        sParams.End.Z -= Character->GetDefaultHalfHeight() / 2;
        UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params, sParams, HitWallAdd,
                                                ECollisionChannel::ECC_WorldStatic, false);
        if(HitWallAdd.GetActor() != nullptr)
        {
            UE_LOG(LogTemp,Warning, TEXT("HangOnWall: error Hit under character"));
            return;
        }
        else
        {
            Character->SetActorLocation(hangLocation, false, nullptr, ETeleportType::None);
            Character->SetActorRotation(hangData.Get()->Rotation, ETeleportType::None);
            SetMovementMode(MOVE_Custom,ECustomMovementMode::ClimbingWall);
        }
    }
}

void UBaseMovementComponent::ClimbWall()///TODO replace teleports and setmovementmode with montages
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
            case EClimbTraceResult::SuccessStand:
            {
                if(ClimbWallMontage != nullptr)
                //TODO calculate playRate with agility and when done set movement mode
                    Character->PlayAnimMontage(ClimbWallMontage,1);
                else
                {
                    Character->SetActorLocation(AnimEndLocation);
                    SetMovementMode(EMovementMode::MOVE_Walking);
                }
                return;
            }
            case EClimbTraceResult::SuccessCrouch:
            {
                Character->GetCapsuleComponent()->SetCapsuleHalfHeight(CrouchedHalfHeight);
                bWantsToCrouch = true;
                if(ClimbWallCrouchedMontage != nullptr)
                //TODO calculate playRate with agility and when done set movement mode
                    Character->PlayAnimMontage(ClimbWallCrouchedMontage,1);
                else
                {
                    Character->SetActorLocation(AnimEndLocation);
                    SetMovementMode(EMovementMode::MOVE_Walking);
                }
                return;
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

bool UBaseMovementComponent::CheckCanHangAgain()
{
    if(!bCanHangAgain)
    {
        auto NeededVelocity = sqrtf(Character->GetCapsuleComponent()->GetScaledCapsuleRadius() / 
                            -GetGravityZ() * 2) * 1000;
        auto CurrentVelocity = fabsf(Character->GetVelocity().Z);
        UE_LOG(LogTemp,Warning, TEXT("CheckCanHangAgain: NeededVelocity %f"), NeededVelocity)
        UE_LOG(LogTemp,Warning, TEXT("CheckCanHangAgain: CurrentVelocity %f"), CurrentVelocity)
        bCanHangAgain = CurrentVelocity >= NeededVelocity;
    }
    if(!bCanHangAgain)
        UE_LOG(LogTemp,Warning, TEXT("CheckCanHangAgain: cant"))
    return bCanHangAgain;
}

void UBaseMovementComponent::JumpOffWall()///TODO define
{
    //arguments are axis directions
    //called when hanging on wall and pressing jump key
    //change move mode to fall, make char accelerate upwards in a desired direction
    //todo canhangagain true
    // todo сделать чтобы направления были с учетом поворота камеры
    UE_LOG(LogTemp,Warning, TEXT("JumpOffWall: doing jmp off"))
    if ( CharacterOwner && CharacterOwner->CanJump() )
	{
		// Don't jump if we can't move up/down.
		if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
		{
			Velocity.Z = FMath::Max(Velocity.Z, JumpZVelocity);
			SetMovementMode(MOVE_Falling);
		}
	}
}

void UBaseMovementComponent::OnMovementModeChanged(EMovementMode prevMode, uint8 prevCustomMode)
{
    if (MovementMode == MOVE_Swimming && IsCrouching())
    {
        UnCrouch();
    }

    if (prevMode == MOVE_Falling)
    {
        bNotifyApex = true;
    }

    if (prevMode == MOVE_Swimming && IsCrouching())
    {
        UnCrouch();
    }
    if (MovementMode == MOVE_Falling || MovementMode == MOVE_Custom)
    {
        fallLocationHeight = GetOwner()->GetActorLocation().Z;
        if(IsCrouching())
        {
            Character->GetCapsuleComponent()->SetCapsuleHalfHeight(Character->GetDefaultHalfHeight());
            bWantsToCrouch = false;
            bWasCrouching = true;
        }
    }
    if (MovementMode == MOVE_Walking)
    {
        if(bWasCrouching)
        {
            Character->GetCapsuleComponent()->SetCapsuleHalfHeight(CrouchedHalfHeight);
            bWantsToCrouch = true;
            bWasCrouching = false;
        }
    }
    if (prevMode == MOVE_Falling && MovementMode == MOVE_Walking)
    {

        if(!bFallDamageInvul)
            FallDamageCheck();
        bCanHangAgain = true;
    }

    if (prevMode == MOVE_Walking)
        bIsSprinting = false;

    Super::OnMovementModeChanged(prevMode, prevCustomMode);

    //////////////debug

    //FString PMM = FString::FromInt(prevMode);
    //FString CMM = FString::FromInt(MovementMode);
    //UE_LOG(LogTemp, Warning, TEXT("Previous %s"), *PMM);
    //UE_LOG(LogTemp, Warning, TEXT("Current %s"), *CMM);


}

float UBaseMovementComponent::GetMaxSpeed() const
{
    switch (MovementMode)
    {
    case MOVE_Walking:
    case MOVE_NavWalking:
    {
        if (IsCrouching() && bIsRunning)
            return MaxRunSpeedCrouched;
        else if (IsCrouching() && !bIsRunning)
            return MaxWalkSpeedCrouched;
        else if (!bWantToSprint && !bIsRunning)
            return MaxWalkSpeed;
        else if (!bWantToSprint && bIsRunning)
            return MaxRunSpeed;
        else if (bWantToSprint && bIsRunning)
            return MaxSprintSpeed;
    }
    case MOVE_Falling:
    {
        return MaxWalkSpeed;
        break;
    }
    case MOVE_Swimming:
    {
        return MaxSwimSpeed;
        break;
    }
    case MOVE_Flying:
    {
        return MaxFlySpeed;
        break;
    }
    case MOVE_Custom:
    {
        return MaxCustomMovementSpeed;
        break;
    }
    case MOVE_None:
    default:
        return 0.f;
    }
}

void UBaseMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
    Super::PhysWalking(deltaTime, Iterations);

    if (!bIsSprinting && Velocity.Size() > MaxWalkSpeedCrouched + 1.f && bWantToSprint)
        bIsSprinting = true;
    else if(bIsSprinting)
        bIsSprinting = false;
    if(bIsSprinting)
        UE_LOG(LogTemp, Warning, TEXT("Sprinttt"));
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
        case(ECustomMovementMode::ClimbingLadder):
        case(ECustomMovementMode::ClimbingWall):
        case(ECustomMovementMode::ClimbingLedge):
        case(ECustomMovementMode::Slide):
        case(ECustomMovementMode::Dodge):
        default:
            return;
    }
}

//проверка получит ли персонаж урон от падения и сколько
//TODO добавить реализацию получения урона
bool UBaseMovementComponent::FallDamageCheck()
{
    const float difference = fallLocationHeight - GetOwner()->GetActorLocation().Z;
    fallLocationHeight = 0;
    if (fallHeightThreshold < difference)
    {
        ApplyFallDamage(difference);
        return true;
    }
    else
    return false;
}

void UBaseMovementComponent::ApplyFallDamage(float FallDistance)
{
    float damage = 0;
     //TODO добавить в персонажа геттер максимального конечного хп
    damage = Cast<ACharacterBase>(GetOwner())->GetAttributesComponent()->MaxHealth;
     //TODO учесть модификатор ловкости
    //max fall height calculation
    const int32 DexMod = UAttributesHelper::GetModifier(
        Character->GetAttributesComponent()->CurrentAttributes,EAttributes::Dex); //TODO temp
    auto MaxFallHeightMod = 1 + DexMod;//temp
    if (FallDistance < MaxfallHeightThresholdBase * MaxFallHeightMod)
        damage *= (FallDistance - fallHeightThreshold) / fallHeightThreshold;
    //UE_LOG(LogTemp, Warning, TEXT("fall damage %f"), damage);
}

void UBaseMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    //для игрока каст в игрового перса
    Character = Cast<ACharacterBase>(PawnOwner);
    check(Character != nullptr && "UBaseMovementComponent: No pawn")
    params.ActorsToIgnore.Add(CharacterOwner);
}

void UBaseMovementComponent::NotifyJumpApex()
{
    /*
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
    //UE_LOG(LogTemp, Warning, TEXT("iswalkablefloor %s"), CurrentFloor.IsWalkableFloor() ? TEXT("true") : TEXT("false"));
    //UE_LOG(LogTemp, Warning, TEXT("floor distance %f"), CurrentFloor.GetDistanceToFloor());
}

void UBaseMovementComponent::Update()
{
    //todo jump velocity calculation with agility(dex)
    //todo animation play rate (dex)
    //todo изменения в возможностях передвижения например перегрузка
	//TODO: рассчитать реальный мод с учетом текущих бафов и 
    // веса снаряжения относительно максимума
    //Max Speed update
    CalculateMaxSpeeds(1);
}

void UBaseMovementComponent::Crouch(bool bClientSimulation)
{
    if(MoveAbilitiesCurrent & EMoveAbilities::CanCrouch)
    {
        Super::Crouch(bClientSimulation);
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("blocked crouch"));
    bWantsToCrouch = false;
}

bool UBaseMovementComponent::DoJump(bool bReplayingMoves)
{
    if(MoveAbilitiesCurrent & EMoveAbilities::CanJump)
        return Super::DoJump(bReplayingMoves);
    UE_LOG(LogTemp, Warning, TEXT("blocked jump"));
    return false;
}

bool UBaseMovementComponent::CanMove()
{
    //todo определить см объявление
    return true;
}
