// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMovementComponent.h"
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
    bCanHangAgain = true;
    ///////////Debug
    MaxWalkSpeedDefault = 200;
    MaxWalkSpeedCrouchedDefault = 150;
    MaxClimbRotationAngle = 50;
    MaxVaultLedgeSize = 85;
    MaxStepHeight = 50;
    MinClimbWallHeight = 80;
    LedgeTraceLength = 70;
    LedgeTraceRadius = 30;
    ///////////End Degbug
    /*
    bUseControllerDesiredRotation
    bOrientRotationToMovement
    CrouchedHalfHeight
    JumpZVelocity
    AirControl
    MaxCustomMovementSpeed
    */
}

void UBaseMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    Character = Cast<ACharacterBase>(PawnOwner);
    check(Character != nullptr && "No pawn")
    // temp (can't climb over the walls that higher than pelvis)
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

void UBaseMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                           FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    //UE_LOG(LogTemp, Warning, TEXT("iswalkablefloor %s"), CurrentFloor.IsWalkableFloor() ? TEXT("true") : TEXT("false"));
    //UE_LOG(LogTemp, Warning, TEXT("floor distance %f"), CurrentFloor.GetDistanceToFloor());
}

void UBaseMovementComponent::Update()
{
    //jump velocity calculation with agility(dex)
    //animation play rate (dex)
    check(Character != nullptr && "No pawn");
    //add a function for setting max/min values for calculations
    //max fall height calculation
    const int32 DexMod = UAttributesHelper::GetModifier(
        Character->GetAttributesComponent()->CurrentAttributes,EAttributes::Dex); //TODO temp
    MaxFallHeightMod = 1 + DexMod;//temp
}

void UBaseMovementComponent::CalculateMaxSpeed(float SpeedModifier)
{
	MaxWalkSpeed = MaxWalkSpeedDefault;
	MaxWalkSpeedCrouched = MaxWalkSpeedCrouchedDefault;
	MaxRunSpeed = MaxRunSpeedDefault * SpeedModifier;
	MaxRunSpeedCrouched = MaxRunSpeedCrouchedDefault * SpeedModifier;
	MaxSprintSpeed = MaxSprintSpeedDefault * SpeedModifier;
	MaxCustomMovementSpeed = MaxWallClimbSpeedDefault * SpeedModifier;
	MaxSwimSpeed = MaxSwimSpeedDefault * SpeedModifier;
	MaxFlySpeed = MaxFlySpeedDefault * SpeedModifier;
}

FTraceSpawnParams UBaseMovementComponent::GetHorizTraceSpawnParams(const bool& IsLow)
{
    const FVector CharLocation = Character->GetActorLocation();
    const FRotator CharRotation = Character->GetActorRotation();
    FTraceSpawnParams sParamsH;
    FQuat Rotation(0, 0, 0, 1);
    Rotation = Character->GetActorRotation().Quaternion();
    sParamsH.Rotation = Rotation;
    LedgeTraceWidth = Character->GetDefaultHalfHeight()/2;
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
            sParamsH.Start = sParamsH.End = CharLocation + FVector(0,0,crouchHeightDiff +
                                                     Character->GetDefaultHalfHeight()/2) +
                                                           HorizLedgeTraceOffset;
        else
            sParamsH.Start = sParamsH.End = CharLocation + 
                                    FVector(0,0,Character->GetDefaultHalfHeight()/2) +
                                     HorizLedgeTraceOffset;
        sParamsH.End += CharRotation.Vector() * LedgeTraceLength;
    }
    return sParamsH;
}

FTraceSpawnParams UBaseMovementComponent::GetVertTraceSpawnParams(const FHitResult &Hit,
                                                                 const bool& IsLow)
{
        const FVector CharLocation = Character->GetActorLocation();
        auto crouchHeightDiff = Character->GetDefaultHalfHeight() - CrouchedHalfHeight;
        //vertical trace
        FTraceSpawnParams sParamsV;
        const FRotator offsetDir(0,Hit.ImpactNormal.Rotation().Yaw - 180,0);
        sParamsV.Start = sParamsV.End = Hit.ImpactPoint + offsetDir.Vector();
        if(IsLow)
        {
            if(IsCrouching())
            {
                sParamsV.Start.Z = CharLocation.Z + crouchHeightDiff + LedgeTraceRadius;
                sParamsV.End.Z = CharLocation.Z + crouchHeightDiff - Character->GetDefaultHalfHeight();
            }
            else
            {
                sParamsV.Start.Z = CharLocation.Z + LedgeTraceRadius;
                sParamsV.End.Z = CharLocation.Z - Character->GetDefaultHalfHeight();
            }
        }
        else
        {
            if(IsCrouching())
            {
                sParamsV.Start.Z = CharLocation.Z + crouchHeightDiff + Character->GetDefaultHalfHeight() +
                        LedgeTraceRadius;
                sParamsV.End.Z = CharLocation.Z + crouchHeightDiff;
            }
            else
            {
                sParamsV.Start.Z = CharLocation.Z + Character->GetDefaultHalfHeight() + LedgeTraceRadius;
                sParamsV.End.Z = CharLocation.Z;
            }
        }
    return sParamsV;
}

bool UBaseMovementComponent::LedgeHeightCheck(const float WallZHeight,
                                            const float charAndWallAngle, 
                                            const bool& IsLow)
{
    const FVector CharLocation = Character->GetActorLocation();
    float minHeight;
    float maxHeight;
    auto crouchHeightDiff = Character->GetDefaultHalfHeight() - CrouchedHalfHeight;
    if(charAndWallAngle < MaxClimbRotationAngle)
    {
    if(IsLow)
    {
        if(IsCrouching())
        {
            minHeight = CharLocation.Z + crouchHeightDiff - Character->GetDefaultHalfHeight() +
                                 MaxStepHeight;
            maxHeight = CharLocation.Z + crouchHeightDiff;
        }
        else
        {
            minHeight = CharLocation.Z - Character->GetDefaultHalfHeight() + MaxStepHeight;
            maxHeight = CharLocation.Z;
        }
    }
    else
    {
        if(IsCrouching())
        {
            minHeight = CharLocation.Z + crouchHeightDiff - Character->GetDefaultHalfHeight() +
                                 Character->GetMovementComponent()->GetMaxJumpHeight();
            maxHeight = CharLocation.Z + crouchHeightDiff + Character->GetDefaultHalfHeight() +
                    MinClimbWallHeight;
        }
        else
        {
            minHeight = CharLocation.Z - Character->GetDefaultHalfHeight() +
                         Character->GetMovementComponent()->GetMaxJumpHeight();
            maxHeight = CharLocation.Z + Character->GetDefaultHalfHeight() + MinClimbWallHeight;
        }
    }
    }
    else
    {
        UE_LOG(LogTemp,Warning, TEXT("LedgeHeightCheck: too large angle"));
        return false;
    }
    //wall height check
    if (WallZHeight > minHeight && WallZHeight < maxHeight)
    {
        return true;
    }
    UE_LOG(LogTemp,Warning, TEXT("LedgeHeightCheck: height is wrong"));
    return false;
}
///add line trace check to climb loc for wall normal
bool UBaseMovementComponent::VaultSpaceCheck(const FHitResult& HorizHit,const FHitResult& VertHit)
{
    //const float SmoothnessThreshold = MaxVaultLedgeSize / tan(GetWalkableFloorAngle());
    const FVector CharLocation = Character->GetActorLocation();
    FQuat Rotation(FRotator(0,HorizHit.Normal.Rotation().Yaw,90));
    FHitResult vaultHit;
    FTraceParams params;
    FTraceSpawnParams sParams;
    sParams.Rotation = Rotation;
    //trace above wall perpendicular to it
    const FRotator baseDir(0,HorizHit.ImpactNormal.Rotation().Yaw - 180,0);
    sParams.Start = FVector(CharLocation.X, CharLocation.Y, VertHit.ImpactPoint.Z +
                            Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 1) +
            baseDir.Vector() *
            Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
    sParams.End = sParams.Start + baseDir.Vector() *
            (MaxVaultLedgeSize - Character->GetCapsuleComponent()->GetScaledCapsuleRadius());
    UTraceManager::CapsuleTraceSingleByChannel(Character->GetWorld(),
                                               Character->GetCapsuleComponent()->GetScaledCapsuleRadius(),
                                               Character->GetCapsuleComponent()->GetScaledCapsuleRadius(),
                                               params,
                                               sParams,
                                               vaultHit,
                                               EWallChannel,
                                               false);
    if(vaultHit.GetActor() == nullptr)
    {
        sParams.Rotation = FQuat();
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
    return false;
}

FVector UBaseMovementComponent::GetClimbStartLoc(const FTraceSpawnParams& startTrace,
                                                const float& maxHeight,
                                                const float& minHeight)
{
    auto CharRotation = Character->GetActorRotation();
    FVector FinalLocation = FVector::ZeroVector;
    FHitResult HitWallH;
    const FTraceParams params;
    FTraceSpawnParams sParamsH = startTrace;
    auto capsuleHalfHeight = (maxHeight - minHeight) / 2;
    auto NumOfIterations = 0;
    auto currentTraceHeight = sParamsH.Start.Z;
    auto currentTraceDistance = 0.f;
    auto prevTraceDistance = 0.f;
    float charBottomHeight;
    if(IsCrouching())
        charBottomHeight = Character->GetActorLocation().Z - CrouchedHalfHeight;
    else
        charBottomHeight = Character->GetActorLocation().Z - Character->GetDefaultHalfHeight();

    ///main loop
    for(;minHeight <= currentTraceHeight <= maxHeight;++NumOfIterations)
    {
        UE_LOG(LogTemp,Warning, TEXT("GetClimbStartLoc: step № %f"),NumOfIterations);
        HitWallH.Reset();
        UTraceManager::CapsuleTraceSingleByChannel(Character->GetWorld(), LedgeTraceRadius,
                                                   capsuleHalfHeight, params, sParamsH, HitWallH,
                                                   EWallChannel, false);

        if(HitWallH.GetActor() == nullptr ||
                FVector::DotProduct(HitWallH.ImpactPoint, CharRotation.Vector()) < 0)
            break;
        if(IsCharacterCapsuleOk(HitWallH.ImpactPoint, false))
            break;
        const auto charAndWallAngle = fabsf(fabsf(CharRotation.Yaw -
             HitWallH.ImpactNormal.Rotation().Yaw) - 180);///TODO add angle check

        UE_LOG(LogTemp,Warning, TEXT("GetClimbStartLoc: behind char angle %f"), 
            FVector::DotProduct(HitWallH.ImpactPoint, Character->GetActorRotation().Vector()));
        currentTraceDistance = FVector::Dist(GetActorLocation(), HitWallH.ImpactPoint);
        UE_LOG(LogTemp,Warning, TEXT("GetClimbStartLoc: hit distance %f"), currentTraceDistance);
        if(NumOfIterations != 0 && currentTraceDistance >= prevTraceDistance)
            break;
        else if(NumOfIterations != 0)
            prevTraceDistance = currentTraceDistance;
        currentTraceHeight = HitWallH.ImpactPoint.Z - charBottomHeight;
        auto minDistToBorder = [currentTraceHeight,maxHeight,minHeight](){if(fabsf(currentTraceHeight-maxHeight) >=
            fabsf(currentTraceHeight-minHeight)) return fabsf(currentTraceHeight-minHeight); else return fabsf(currentTraceHeight-maxHeight);};///TODO kill it
        capsuleHalfHeight = minDistToBorder();
        sParamsH.Start.Z = sParamsH.End.Z = HitWallH.ImpactPoint.Z;
        FinalLocation = HitWallH.ImpactPoint;
    }
    //vert trace
    if(FinalLocation != FVector::ZeroVector)
    {
        ///wall normal direction is suboptimal
        const FRotator offsetDir(0,HitWallH.ImpactNormal.Rotation().Yaw - 180,0);
        FTraceSpawnParams sParamsV;
        sParamsV.Start = sParamsV.End = FinalLocation + offsetDir.Vector();
        sParamsV.Start.Z = maxHeight;
        sParamsV.End.Z = minHeight;
        FHitResult HitWallV;
        UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params,
                                                sParamsV, HitWallV,
                                                EWallChannel, false);
        if(HitWallV.GetActor() == nullptr)
            UE_LOG(LogTemp, Warning, TEXT("GetClimbStartLoc: vert trace miss"))
        else
            FinalLocation.Z = HitWallV.ImpactPoint.Z;
    }
    UE_LOG(LogTemp,Warning, TEXT("GetClimbStartLoc: Number of steps %f"),NumOfIterations);
    return FinalLocation;
}

bool UBaseMovementComponent::CanVaultOrClimb(const bool& IsLow)
{
    UE_LOG(LogTemp,Warning, TEXT("CanVault or climb %s"), IsLow ? TEXT("low") : TEXT("high"));
    const FRotator CharRotation = Character->GetActorRotation();
    // trace parameters define
    FTraceParams params;
    //setting spawn parameters
    FTraceSpawnParams sParamsH = GetHorizTraceSpawnParams(IsLow);
    //get max distance allowed for parkour from char feet
    float maxHeight;
    const float minHeight = MaxStepHeight;
    IsLow ?  maxHeight = Character->GetDefaultHalfHeight(): maxHeight = 
        Character->GetDefaultHalfHeight()*2 + MinClimbWallHeight;
    //horiz and vert trace to find ledge location
    auto LedgeLoc = GetClimbStartLoc(sParamsH,maxHeight,minHeight);

    ///TODO fiiiix next
    FHitResult HitWallH;
    if(HitWallH.GetActor() != nullptr)
    {
        //angle check for wall normal
        auto horizAngle = HitWallH.ImpactNormal.Rotation().Pitch;
        UE_LOG(LogTemp,Warning, TEXT("CanV: Horiz angle %f"), horizAngle);
        FTraceSpawnParams sParamsV = GetVertTraceSpawnParams(HitWallH, IsLow);
        FHitResult HitWallV;
        //vert trace
        UTraceManager::LineTraceSingleByChannel(Character->GetWorld(),
                                                params, 
                                                sParamsV,
                                                HitWallV, 
                                                EWallChannel, 
                                                false);
        if(HitWallV.GetActor() != nullptr)
        {
            //angle check for wall normal
            auto vertAngle = HitWallV.ImpactNormal.Rotation().Pitch - 90;
            UE_LOG(LogTemp,Warning, TEXT("CanV: Vert angle %f"), vertAngle);
            const auto charAndWallAngle = fabsf(fabsf(CharRotation.Yaw - 
                HitWallH.ImpactNormal.Rotation().Yaw) - 180);
            if(LedgeHeightCheck(HitWallV.ImpactPoint.Z, charAndWallAngle, IsLow))
            {
                if(VaultSpaceCheck(HitWallH,HitWallV))
                {
                    UE_LOG(LogTemp,Warning, TEXT("CanVault: doing vault %s"), 
                        IsLow ? TEXT("low") : TEXT("high"));
                    if(DoVault(HitWallH, HitWallV, IsLow))
                        return true;
                }
                UE_LOG(LogTemp,Warning, TEXT("CanVault: not enough space above or behind, checking climb"));
                UE_LOG(LogTemp,Warning, TEXT("CanVault: doing climb %s"), 
                    IsLow ? TEXT("low") : TEXT("high"));
                if(DoClimbLedge(HitWallH, HitWallV, IsLow))
                    return true;
                UE_LOG(LogTemp,Warning, TEXT("CanClimb: can't climb"));
            }
            else
                UE_LOG(LogTemp,Warning, TEXT("CanVault: cant't parkour"));

        }
        else
        {
            UE_LOG(LogTemp,Warning, TEXT("CanVault or climb: No Vertial Actor"));
        }
    }
    else
    {
        UE_LOG(LogTemp,Warning, TEXT("CanVault or climb: No Horizontal Actor"));
    }
    return false;
}

bool UBaseMovementComponent::DoVault(const FHitResult &HorizHit, 
                                    const FHitResult &VertHit, 
                                    const bool& IsLow)///TODO replace hitres with loc
{
    ///TODO add stamina calculation and usage
    // check for available space for anim and set anim location and move there
    const FRotator baseDir(0,HorizHit.ImpactNormal.Rotation().Yaw,0); // vector in the direction opposite to the wall's normal
    //check for available space at the position before anim
    float charPelvisheight;
    if(IsCrouching())
        charPelvisheight = Character->GetActorLocation().Z + 
            Character->GetDefaultHalfHeight() + CrouchedHalfHeight;
    else
        charPelvisheight = Character->GetActorLocation().Z;
    FVector VaultAnimationStartLocation;
    if(VertHit.ImpactPoint.Z - charPelvisheight > 0)
        VaultAnimationStartLocation = FVector(HorizHit.ImpactPoint.X,HorizHit.ImpactPoint.Y,VertHit.ImpactPoint.Z) +
                                              FVector::UpVector + baseDir.Vector() *
            (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 1);
    else
        VaultAnimationStartLocation = FVector(HorizHit.ImpactPoint.X,HorizHit.ImpactPoint.Y,
                                              Character->GetActorLocation().Z) +
            FVector::UpVector + baseDir.Vector() * (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 1);
    const FVector VaultAnimationEndLocation = VaultAnimationStartLocation + FRotator(0, baseDir.Yaw - 180, 0).Vector() *
            ((Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 1) + MaxVaultLedgeSize);

    if(IsEnoughSpace(VaultAnimationStartLocation, VaultAnimationEndLocation) == EClimbTraceResult::SuccessStand)
    {
        SetMovementMode(EMovementMode::MOVE_Custom,ECustomMovementMode::ClimbingLedge);
         ///TODO replace with smoothness
        Character->SetActorLocation(VaultAnimationStartLocation, false, nullptr, ETeleportType::None);
        Character->SetActorRotation(FRotator(0, HorizHit.ImpactNormal.Rotation().Yaw - 180, 0), ETeleportType::None);
        if(IsLow && VaultMontageLow != nullptr)
         ///TODO calculate playRate with agility and when done set movement mode
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
    else
        return false;
}
///TODO replace hitres with loc
bool UBaseMovementComponent::DoClimbLedge(const FHitResult &HorizHit, 
                                            const FHitResult &VertHit, 
                                            const bool& IsLow)
{
    ///TODO add stamina calculation and usage
    // check for available space for anim and set anim location and move there
    // vector in the direction opposite to the wall's normal
    const FRotator baseDir(0,HorizHit.ImpactNormal.Rotation().Yaw,0);
    // check for available space at the position before anim
    auto ClimbAnimHorizDistance = 50;///TODO temp
    float charPelvisheight;
    float charBottomHeight;
    if(IsCrouching())
    {
        charPelvisheight = Character->GetActorLocation().Z - Character->GetDefaultHalfHeight() + CrouchedHalfHeight;
        charBottomHeight = charPelvisheight - CrouchedHalfHeight;
    }
    else
    {
        charPelvisheight = Character->GetActorLocation().Z;
        charBottomHeight = charPelvisheight - Character->GetDefaultHalfHeight();
    }
    FVector ClimbledgeAnimationStartLocation;
    if(VertHit.ImpactPoint.Z - charPelvisheight > 0)
        ClimbledgeAnimationStartLocation = FVector(HorizHit.ImpactPoint.X,HorizHit.ImpactPoint.Y,VertHit.ImpactPoint.Z) +
                                              FVector::UpVector + baseDir.Vector() *
            (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 1);
    else
        ClimbledgeAnimationStartLocation = FVector(HorizHit.ImpactPoint.X,HorizHit.ImpactPoint.Y,
                                              Character->GetActorLocation().Z) +
            FVector::UpVector + baseDir.Vector() * (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 1);
    const FVector ClimbledgeAnimationEndLocation = ClimbledgeAnimationStartLocation +
            FVector(0,0,Character->GetDefaultHalfHeight()) +
            FRotator(0, baseDir.Yaw - 180, 0).Vector() *
            ((Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 1) + ClimbAnimHorizDistance);

    // check for available space at the start position before anim
    switch(IsEnoughSpace(ClimbledgeAnimationStartLocation, ClimbledgeAnimationEndLocation))
    {
        case EClimbTraceResult::SuccessStand:
        {
            SetMovementMode(EMovementMode::MOVE_Custom,ECustomMovementMode::ClimbingLedge);
             //TODO replace with smooth motion
            Character->SetActorLocation(ClimbledgeAnimationStartLocation, false, nullptr, ETeleportType::None);
            Character->SetActorRotation(FRotator(0, HorizHit.ImpactNormal.Rotation().Yaw - 180, 0), ETeleportType::None);
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
            Character->SetActorRotation(FRotator(0, HorizHit.ImpactNormal.Rotation().Yaw - 180, 0), ETeleportType::None);
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
        case EClimbTraceResult::FailAtEnd:
        case EClimbTraceResult::FailAtStart:
        return false;
    }
}


EClimbTraceResult UBaseMovementComponent::IsEnoughSpace(const FVector& StartLocation,
                                                        const FVector& EndLocation,
                                                        const ECollisionChannel& startCC, 
                                                        const ECollisionChannel& endCC)
{
    //check at the start
    if(IsCharacterCapsuleOk(StartLocation,IsCrouching(),true,startCC) ||
            IsCharacterCapsuleOk(StartLocation,IsCrouching(),false,startCC))
    {
        UE_LOG(LogTemp,Warning, TEXT("not enough space for anim start"));
        return EClimbTraceResult::FailAtStart;
    }

    //check for available space at the end of anim (may not be needed)
    if(!EndLocation.IsZero())
    {
        if(IsCharacterCapsuleOk(EndLocation,IsCrouching(),true,endCC) ||
                IsCharacterCapsuleOk(EndLocation,IsCrouching(),false,endCC))
        {
            UE_LOG(LogTemp,Warning, TEXT("not enough space for stand anim end"));
            auto tempEnd = EndLocation + FVector(0,0,-Character->GetDefaultHalfHeight() + CrouchedHalfHeight);
            if(IsCharacterCapsuleOk(tempEnd,true,true,endCC) ||
                    IsCharacterCapsuleOk(tempEnd,true,false,endCC))
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
    if(!H2L)
    {
        auto buffer = sParams.Start;
        sParams.Start = sParams.End;
        sParams.End = buffer;
    }

    //do trace check
    FQuat Rotation(0, 0, 0, 1);
    Rotation = Character->GetActorRotation().Quaternion();
    FHitResult HitWall;
    FTraceParams params;
    params.ActorsToIgnore.Add(CharacterOwner);
    UTraceManager::SphereTraceSingleByChannel(Character->GetWorld(),
                                               Character->GetCapsuleComponent()->GetScaledCapsuleRadius(),
                                               params, sParams, HitWall,
                                               CC, false);
    return HitWall.GetActor() == nullptr;
}

FVector UBaseMovementComponent::FindHangLoc(const FHitResult &HitH, const FHitResult &HitV) const
{
    const FVector HangLocation = FVector(HitH.ImpactPoint.X, HitH.ImpactPoint.Y, HitV.ImpactPoint.Z -
                                         Character->GetDefaultHalfHeight() + HangLocationVertOffset) +
                                         FRotator(0,HitH.Normal.Rotation().Yaw,0).Vector() *
                                         (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() +
                                          HangLocationHorizOffset);
    return HangLocation;
}

FRotator UBaseMovementComponent::FindHangRot(const FHitResult &HitH) const
{
    const FRotator HangRotation = FRotator(0,HitH.ImpactNormal.Rotation().Yaw - 180,0);
    return HangRotation;
}

void UBaseMovementComponent::IsClimbableWall(FHitResult HitWallH, bool CheckCharacterRotation)
{
    if(CheckCharacterRotation)
    {
        if(fabsf(fabsf(Character->GetActorRotation().Yaw -
                       HitWallH.ImpactNormal.Rotation().Yaw) - 180) > MaxClimbRotationAngle)
            return;
    }
    float ClimbWallTraceHeight = 2 * Character->GetCapsuleComponent()->GetScaledCapsuleRadius();//TODO temp
    FQuat Rotation(0, 0, 0, 1);
    Rotation = Character->GetActorRotation().Quaternion();
    FTraceParams params;
    FTraceSpawnParams sParams;
    params.ActorsToIgnore.Add(Character);
    sParams.Rotation = Rotation;
    if(!IsRootOrTeleport() && HitWallH.GetActor() != nullptr)
    {
        UE_LOG(LogTemp,Warning, TEXT("Wall Climb H trace hit"));
        sParams.Start = sParams.End = HitWallH.ImpactPoint;
        sParams.Start += FVector(0,0,ClimbWallTraceHeight);
        FHitResult HitWallV;
        UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params, sParams, HitWallV, EWallChannel,
                                                false);
        if(HitWallV.GetActor() != nullptr)
        {
            UE_LOG(LogTemp,Warning, TEXT("Wall Climb V trace hit"));
            if(GetWalkableFloorAngle() > 90 - HitWallV.Normal.Rotation().Pitch)
            {
                UE_LOG(LogTemp,Warning, TEXT("Floor angle less than walkable"));
                if(IsEnoughSpace(FindHangLoc(HitWallH,HitWallV)) == EClimbTraceResult::SuccessStand)
                {
                    UE_LOG(LogTemp,Warning, TEXT("Wall Climb Enough space for capsule"));
                    FHitResult HitWallAdd;
                    sParams.Start = sParams.End = FindHangLoc(HitWallH,HitWallV) + FVector(0,0,
                                                                         -Character->GetDefaultHalfHeight());
                    sParams.End.Z -= MinClimbWallHeight;
                    UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params, sParams, HitWallAdd,
                                                            ECollisionChannel::ECC_WorldStatic, false);
                    if(HitWallAdd.GetActor() != nullptr)
                    {
                        UE_LOG(LogTemp,Warning, TEXT("Climb Wall Hit under character"));
                        return;
                    }
                    else                                             ///TODO rework
                    {
                        sParams.Start = sParams.End = FindHangLoc(HitWallH,HitWallV)  + FVector(0,0,Character->GetDefaultHalfHeight() -
                                Character->GetCapsuleComponent()->GetScaledCapsuleRadius()) + UKismetMathLibrary::GetRightVector(FindHangRot(HitWallH)) *
                                Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
                        sParams.End += FindHangRot(HitWallH).Vector() * (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 2);//delta
                        UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params, sParams, HitWallAdd, EWallChannel, false);
                        if(HitWallAdd.GetActor() != nullptr && fabsf(HitWallH.ImpactNormal.Rotation().Yaw - HitWallAdd.ImpactNormal.Rotation().Yaw) < 1)
                        {
                            UE_LOG(LogTemp,Warning, TEXT("Climb Wall right"));
                            HitWallAdd.Reset();
                            sParams.Start = sParams.End = FindHangLoc(HitWallH,HitWallV)  + FVector(0,0,Character->GetDefaultHalfHeight() -
                                Character->GetCapsuleComponent()->GetScaledCapsuleRadius()) + UKismetMathLibrary::GetRightVector(FindHangRot(HitWallH)) *
                                -Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
                            sParams.End += FindHangRot(HitWallH).Vector() * (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 2);//delta
                            UTraceManager::LineTraceSingleByChannel(Character->GetWorld(), params, sParams, HitWallAdd, EWallChannel, false);
                            if(HitWallAdd.GetActor() != nullptr && fabsf(HitWallH.ImpactNormal.Rotation().Yaw - HitWallAdd.ImpactNormal.Rotation().Yaw) < 1)
                            {
                                UE_LOG(LogTemp,Warning, TEXT("Climb Wall left"));
                                Character->SetActorLocation(FindHangLoc(HitWallH,HitWallV), false, nullptr, ETeleportType::None);
                                Character->SetActorRotation(FindHangRot(HitWallH), ETeleportType::None);
                                SetMovementMode(MOVE_Custom,ECustomMovementMode::ClimbingWall);
                                return;
                            }
                        }
                        else
                           UE_LOG(LogTemp,Warning, TEXT("Climb Wall Bad angle or location"));
                    }
                }
            }
        }
    }
}

void UBaseMovementComponent::ClimbWall()///TODO replace teleports and setmovementmode with montages
{
    if(!IsRootOrTeleport())
    {
        float AnimVertMovement = Character->GetDefaultHalfHeight() * 2 - HangLocationVertOffset +1;//TODO temp
        float AnimHorizMovement = Character->GetCapsuleComponent()->GetScaledCapsuleRadius() * 2; //TODO temp
        const FVector AnimEndLocation = Character->GetActorLocation() + FVector(0,0,AnimVertMovement) +
                Character->GetActorRotation().Vector() * AnimHorizMovement;
        switch(IsEnoughSpace(Character->GetActorLocation(),AnimEndLocation))
        {
            case EClimbTraceResult::SuccessStand:
            {
                Character->SetActorLocation(AnimEndLocation, false, nullptr, ETeleportType::None);
                SetMovementMode(EMovementMode::MOVE_Walking);
                break;
            }
            case EClimbTraceResult::SuccessCrouch:
            {
                Character->SetActorLocation(AnimEndLocation + FVector(0,0, - Character->GetDefaultHalfHeight() +
                                            CrouchedHalfHeight), false, nullptr, ETeleportType::None);
                Character->GetCapsuleComponent()->SetCapsuleHalfHeight(CrouchedHalfHeight);
                bWantsToCrouch = true;
                SetMovementMode(EMovementMode::MOVE_Walking);
                break;
            default:
                    return;
            }

        }
    }
}

void UBaseMovementComponent::GetOffWall()
{
    if(!IsRootOrTeleport())
        SetMovementMode(MOVE_Falling);
}

void UBaseMovementComponent::ClimbDownWall()/// TODO doesn't work
{
    float SphereTraceDelta = 30;//TODO temp
    FQuat Rotation(0, 0, 0, 1);
    Rotation = Character->GetActorRotation().Quaternion();
    FHitResult HitWallH;
    FTraceParams params;
    FTraceSpawnParams sParams;
    params.ActorsToIgnore.Add(Character);
    sParams.Rotation = Rotation;
    if(IsCrouching())
        sParams.Start = sParams.End = Character->GetActorLocation() + 
            FVector(0,0, -CrouchedHalfHeight + Character->GetCapsuleComponent()->GetScaledCapsuleRadius());
    else
        sParams.Start = sParams.End = Character->GetActorLocation() + 
            FVector(0,0, -Character->GetDefaultHalfHeight() + Character->GetCapsuleComponent()->GetScaledCapsuleRadius());
    UTraceManager::SphereTraceSingleByChannel(Character->GetWorld(),
                                            Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + SphereTraceDelta,
                                            params, 
                                            sParams, 
                                            HitWallH, 
                                            EWallChannel, 
                                            false);
    IsClimbableWall(HitWallH,false);
}

void UBaseMovementComponent::HangOnWall()
{
    float ClimbWallTracelenght = 70;//TODO temp
    FQuat Rotation(0, 0, 0, 1);
    Rotation = Character->GetActorRotation().Quaternion();
    FHitResult HitWallH;
    FTraceParams params;
    FTraceSpawnParams sParams;
    params.ActorsToIgnore.Add(Character);
    sParams.Start = Character->GetActorLocation() + FVector(0,0,Character->GetDefaultHalfHeight() - Character->GetCapsuleComponent()->GetScaledCapsuleRadius());
    sParams.End = sParams.Start + Character->GetActorRotation().Vector() * ClimbWallTracelenght;
    sParams.Rotation = Rotation;
    UTraceManager::CapsuleTraceSingleByChannel(Character->GetWorld(), Character->GetCapsuleComponent()->GetScaledCapsuleRadius(), Character->GetCapsuleComponent()->GetScaledCapsuleRadius(), params, sParams, HitWallH, EWallChannel, false);
    IsClimbableWall(HitWallH,true);
}

void UBaseMovementComponent::JumpOffWall()///TODO define
{
    //arguments are axis directions
    //called when hanging on wall and pressing jump key
    //change move mode to fall, make char accelerate upwards in a desired direction
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
    if (MovementMode == MOVE_Falling)
    {
        fallLocationHeight = GetOwner()->GetActorLocation().Z;
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

void UBaseMovementComponent::PhysCustom(float deltaTime, int32 Iterations)///TODO
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
//TODO
//добавить реализацию получения урона
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
    if (FallDistance < MaxfallHeightThresholdBase * MaxFallHeightMod)
        damage *= (FallDistance - fallHeightThreshold) / fallHeightThreshold;
    //UE_LOG(LogTemp, Warning, TEXT("fall damage %f"), damage);
}
