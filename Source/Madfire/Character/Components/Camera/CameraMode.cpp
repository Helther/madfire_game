#include "CameraMode.h"
#include "PlayerPawnCameraComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CameraBlockingVolume.h"


FCameraModeView::FCameraModeView() :
    Location(ForceInit),
    Rotation(ForceInit),
    ControlRotation(ForceInit),
    FieldOfView(CAMERA_DEFAULT_FOV)
{}

void FCameraModeView::Blend(const FCameraModeView& Other, float OtherWeight)
{
    if (OtherWeight <= 0.0f)
    {
        return;
    }
    else if (OtherWeight >= 1.0f)
    {
        *this = Other;
        return;
    }

    Location = FMath::Lerp(Location, Other.Location, OtherWeight);

    const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
    Rotation = Rotation + (OtherWeight * DeltaRotation);

    const FRotator DeltaControlRotation = (Other.ControlRotation - ControlRotation).GetNormalized();
    ControlRotation = ControlRotation + (OtherWeight * DeltaControlRotation);

    FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}

UCameraMode::UCameraMode()
{
    FieldOfView = CAMERA_DEFAULT_FOV;
    ViewPitchMin = CAMERA_DEFAULT_PITCH_MIN;
    ViewPitchMax = CAMERA_DEFAULT_PITCH_MAX;

    BlendTime = 0.5f;
    BlendFunction = ECameraModeBlendFunction::EaseOut;
    BlendExponent = 4.0f;
    BlendAlpha = 1.0f;
    BlendWeight = 1.0f;
    TargetOffsetCurve = nullptr;

    PenetrationAvoidanceWhiskers.Add(FPenetrationAvoidanceWhisker(FRotator(+00.0f, +00.0f, 0.0f), 1.00f, 1.00f, 14.f, 0));
    PenetrationAvoidanceWhiskers.Add(FPenetrationAvoidanceWhisker(FRotator(+00.0f, +16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
    PenetrationAvoidanceWhiskers.Add(FPenetrationAvoidanceWhisker(FRotator(+00.0f, -16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
    PenetrationAvoidanceWhiskers.Add(FPenetrationAvoidanceWhisker(FRotator(+00.0f, +32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
    PenetrationAvoidanceWhiskers.Add(FPenetrationAvoidanceWhisker(FRotator(+00.0f, -32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
    PenetrationAvoidanceWhiskers.Add(FPenetrationAvoidanceWhisker(FRotator(+20.0f, +00.0f, 0.0f), 1.00f, 1.00f, 00.f, 4));
    PenetrationAvoidanceWhiskers.Add(FPenetrationAvoidanceWhisker(FRotator(-20.0f, +00.0f, 0.0f), 0.50f, 0.50f, 00.f, 4));
}

AActor* UCameraMode::GetTargetActor() const
{
    auto cameraComponent = CastChecked<UPlayerPawnCameraComponent>(GetOuter());
    check(cameraComponent);
    return cameraComponent->GetTargetActor();
}

UWorld* UCameraMode::GetWorld() const
{
    return GetOuter()->GetWorld();
}

FVector UCameraMode::GetPivotLocation() const
{
    const AActor* TargetActor = GetTargetActor();
    check(TargetActor);

    if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
    {
        // Height adjustments for characters to account for crouching.
        if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
        {
            const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
            check(TargetCharacterCDO);

            const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
            check(CapsuleComp);

            const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
            check(CapsuleCompCDO);

            const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
            const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
            const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight;
            return TargetCharacter->GetActorLocation() + (FVector::UpVector * HeightAdjustment);
        }

        return TargetPawn->GetPawnViewLocation();
    }

    return TargetActor->GetActorLocation();
}

FRotator UCameraMode::GetPivotRotation() const
{
    const AActor* TargetActor = GetTargetActor();
    check(TargetActor);

    if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
    {
        return TargetPawn->GetViewRotation();
    }

    return TargetActor->GetActorRotation();
}

void UCameraMode::UpdateCrouchOffset(float DeltaTime)
{
    // set crouch offset target
    if (const ACharacter *TargetCharacter = Cast<ACharacter>(GetTargetActor()))
    {
        CrouchOffsetBlendPct = 0.0f;
        InitialCrouchOffset  = CurrentCrouchOffset;
        if (TargetCharacter->bIsCrouched)
        {
            const ACharacter *TargetCharacterCDO =
              TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
            const float CrouchedHeightAdjustment =
              TargetCharacterCDO->CrouchedEyeHeight - TargetCharacterCDO->BaseEyeHeight;

            TargetCrouchOffset = FVector(0.f, 0.f, CrouchedHeightAdjustment);
        }
        else
            TargetCrouchOffset = FVector::ZeroVector;
    }
    // update crouch offset
    if (CrouchOffsetBlendPct < 1.0f)
    {
        CrouchOffsetBlendPct = FMath::Min(
          CrouchOffsetBlendPct + DeltaTime * CrouchOffsetBlendMultiplier, 1.0f);
        CurrentCrouchOffset = FMath::InterpEaseInOut(
          InitialCrouchOffset, TargetCrouchOffset, CrouchOffsetBlendPct, 1.0f);
    }
    else
    {
        CurrentCrouchOffset  = TargetCrouchOffset;
        CrouchOffsetBlendPct = 1.0f;
    }
}

void UCameraMode::UpdateZoomOffset(float DeltaTime)
{
    CameraZoomBlendPct = 0.0f;
    InitialZoomOffset  = CurrentZoomOffset;
    if (CameraZoomBlendPct < 1.0f)
    {
        CameraZoomBlendPct =
          FMath::Min(CameraZoomBlendPct + DeltaTime * ZoomBlendMultiplier, 1.0f);
        CurrentZoomOffset = FMath::InterpEaseInOut(InitialZoomOffset, TargetZoomOffset,
                                                   CameraZoomBlendPct, 1.0f);
    }
    else
    {
        CurrentZoomOffset  = TargetZoomOffset;
        CameraZoomBlendPct = 1.0f;
    }
}


void UCameraMode::Initialize() 
{
    CameraDistanceDefault = TargetOffsetCurve->GetVectorValue(0).X;
    CameraDistanceMax = CameraDistanceDefault * CameraDistanceMaxMod;
}

void UCameraMode::UpdateCameraMode(float DeltaTime)
{
    UpdateCrouchOffset(DeltaTime);
    UpdateZoomOffset(DeltaTime);

    FVector PivotLocation  = GetPivotLocation() + CurrentCrouchOffset;
    FRotator PivotRotation = GetPivotRotation();

    PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

    const FVector TargetOffset = TargetOffsetCurve->GetVectorValue(PivotRotation.Pitch);
    View.Rotation = PivotRotation;
    View.Location = PivotLocation + PivotRotation.RotateVector(TargetOffset + CurrentZoomOffset) + CurrentCrouchOffset;
    View.ControlRotation = View.Rotation;
    View.FieldOfView = FieldOfView;

    UpdatePreventPenetration(DeltaTime);

    if (BlendTime > 0.0f)
    {
        BlendAlpha += (DeltaTime / BlendTime);
        BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
    }
    else
    {
        BlendAlpha = 1.0f;
    }

    const float Exponent = (BlendExponent > 0.0f) ? BlendExponent : 1.0f;

    switch (BlendFunction)
    {
    case ECameraModeBlendFunction::Linear:
        BlendWeight = BlendAlpha;
        break;

    case ECameraModeBlendFunction::EaseIn:
        BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
        break;

    case ECameraModeBlendFunction::EaseOut:
        BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
        break;

    case ECameraModeBlendFunction::EaseInOut:
        BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
        break;

    default:
        UE_LOG(LogTemp, Error, TEXT("UCameraMode: UpdateCameraMode: wrong BlendFunction"));
        break;
    }
}

void UCameraMode::UpdatePreventPenetration(float DeltaTime)
{
    if (!PreventPenetration)
        return;

    AActor *TargetActor = GetTargetActor();

    APawn *TargetPawn             = Cast<APawn>(TargetActor);
    AController *TargetController = TargetPawn ? TargetPawn->GetController() : nullptr;


    const UPrimitiveComponent *PPActorRootComponent =
      Cast<UPrimitiveComponent>(TargetPawn->GetRootComponent());
    if (PPActorRootComponent)
    {
        // Attempt at picking SafeLocation automatically, so we reduce camera translation when aiming.
        // Our camera is our reticle, so we want to preserve our aim and keep that as steady and smooth as possible.
        // Pick closest point on capsule to our aim line.
        FVector ClosestPointOnLineToCapsuleCenter;
        FVector SafeLocation = TargetActor->GetActorLocation();
        FMath::PointDistToLine(SafeLocation, View.Rotation.Vector(), View.Location,
                               ClosestPointOnLineToCapsuleCenter);

        // Adjust Safe distance height to be same as aim line, but within capsule.
        float const PushInDistance =
          PenetrationAvoidanceWhiskers[0].Extent + CollisionPushOutDistance;
        float const MaxHalfHeight =
          TargetActor->GetSimpleCollisionHalfHeight() - PushInDistance;
        SafeLocation.Z =
          FMath::Clamp(ClosestPointOnLineToCapsuleCenter.Z,
                       SafeLocation.Z - MaxHalfHeight, SafeLocation.Z + MaxHalfHeight);

        float DistanceSqr;
        PPActorRootComponent->GetSquaredDistanceToCollision(
          ClosestPointOnLineToCapsuleCenter, DistanceSqr, SafeLocation);
        // Push back inside capsule to avoid initial penetration when doing line checks.
        if (PenetrationAvoidanceWhiskers.Num() > 0)
        {
            SafeLocation +=
              (SafeLocation - ClosestPointOnLineToCapsuleCenter).GetSafeNormal()
              * PushInDistance;
        }

        // Then aim line to desired camera position
        bool const SingleRayPenetrationCheck = !DoPredictiveAvoidance;
        PreventCameraPenetration(*TargetActor, SafeLocation, View.Location, DeltaTime,
                                 AimLineToDesiredPosBlockedPct,
                                 SingleRayPenetrationCheck);
    }
}

void UCameraMode::PreventCameraPenetration(AActor const &ViewTarget,
                                               FVector const &SafeLoc, FVector &CameraLoc,
                                               float const &DeltaTime,
                                               float &DistBlockedPct, bool SingleRayOnly)
{

    float HardBlockedPct = DistBlockedPct;
    float SoftBlockedPct = DistBlockedPct;

    FVector BaseRay = CameraLoc - SafeLoc;
    FRotationMatrix BaseRayMatrix(BaseRay.Rotation());
    FVector BaseRayLocalUp, BaseRayLocalFwd, BaseRayLocalRight;

    BaseRayMatrix.GetScaledAxes(BaseRayLocalFwd, BaseRayLocalRight, BaseRayLocalUp);

    float DistBlockedPctThisFrame = 1.f;

    int32 const NumRaysToShoot = SingleRayOnly
      ? FMath::Min(1, PenetrationAvoidanceWhiskers.Num())
      : PenetrationAvoidanceWhiskers.Num();
    FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(CameraPen), false,
                                       nullptr /*PlayerCamera*/);

    SphereParams.AddIgnoredActor(&ViewTarget);

    FCollisionShape SphereShape = FCollisionShape::MakeSphere(0.f);
    UWorld *World               = GetWorld();

    for (int32 RayIdx = 0; RayIdx < NumRaysToShoot; ++RayIdx)
    {
        FPenetrationAvoidanceWhisker &Whisker = PenetrationAvoidanceWhiskers[RayIdx];
        if (Whisker.FramesUntilNextTrace <= 0)
        {
            // calc ray target
            FVector RayTarget;
            {
                FVector RotatedRay =
                  BaseRay.RotateAngleAxis(Whisker.AdjustmentRot.Yaw, BaseRayLocalUp);
                RotatedRay = RotatedRay.RotateAngleAxis(Whisker.AdjustmentRot.Pitch,
                                                        BaseRayLocalRight);
                RayTarget  = SafeLoc + RotatedRay;
            }

            // cast for world and pawn hits separately.  this is so we can safely ignore the
            // camera's target pawn
            SphereShape.Sphere.Radius = Whisker.Extent;
            ECollisionChannel TraceChannel =
              ECC_Camera; //(Whisker.PawnWeight > 0.f) ? ECC_Pawn : ECC_Camera;

            // do multi-line check to make sure the hits we throw out aren't
            // masking real hits behind (these are important rays).

            // MT-> passing camera as actor so that camerablockingvolumes know when it's the camera doing traces
            FHitResult Hit;
            const bool bHit =
              World->SweepSingleByChannel(Hit, SafeLoc, RayTarget, FQuat::Identity,
                                          TraceChannel, SphereShape, SphereParams);
            Whisker.FramesUntilNextTrace = Whisker.TraceInterval;

            const AActor *HitActor = Hit.GetActor();

            if (bHit && HitActor)
            {
                bool IgnoreHit = false;

                // Ignore CameraBlockingVolume hits that occur in front of the ViewTarget.
                if (!IgnoreHit && HitActor->IsA<ACameraBlockingVolume>())
                {
                    const FVector ViewTargetForwardXY =
                      ViewTarget.GetActorForwardVector().GetSafeNormal2D();
                    const FVector ViewTargetLocation = ViewTarget.GetActorLocation();
                    const FVector HitOffset          = Hit.Location - ViewTargetLocation;
                    const FVector HitDirectionXY     = HitOffset.GetSafeNormal2D();
                    const float DotHitDirection =
                      FVector::DotProduct(ViewTargetForwardXY, HitDirectionXY);
                    if (DotHitDirection > 0.0f)
                    {
                        IgnoreHit = true;
                        // Ignore this CameraBlockingVolume on the remaining sweeps.
                        SphereParams.AddIgnoredActor(HitActor);
                    }
                }

                if (!IgnoreHit)
                {
                    float const Weight = Cast<APawn>(Hit.GetActor()) ? Whisker.PawnWeight
                                                                     : Whisker.WorldWeight;
                    float NewBlockPct  = Hit.Time;
                    NewBlockPct += (1.f - NewBlockPct) * (1.f - Weight);

                    // Recompute blocked pct taking into account pushout distance.
                    NewBlockPct =
                      ((Hit.Location - SafeLoc).Size() - CollisionPushOutDistance)
                      / (RayTarget - SafeLoc).Size();
                    DistBlockedPctThisFrame =
                      FMath::Min(NewBlockPct, DistBlockedPctThisFrame);

                    // This Whisker got a hit, so do another trace next frame
                    Whisker.FramesUntilNextTrace = 0;

                }
            }

            if (RayIdx == 0)
            {
                // don't interpolate toward this one, snap to it
                // assumes ray 0 is the center/main ray
                HardBlockedPct = DistBlockedPctThisFrame;
            }
            else
            {
                SoftBlockedPct = DistBlockedPctThisFrame;
            }
        }
        else
        {
            --Whisker.FramesUntilNextTrace;
        }
    }

    if (DistBlockedPct < DistBlockedPctThisFrame)
    {
        // interpolate smoothly out
        if (PenetrationBlendOutTime > DeltaTime)
        {
            DistBlockedPct = DistBlockedPct
              + DeltaTime / PenetrationBlendOutTime
                * (DistBlockedPctThisFrame - DistBlockedPct);
        }
        else
        {
            DistBlockedPct = DistBlockedPctThisFrame;
        }
    }
    else
    {
        if (DistBlockedPct > HardBlockedPct)
        {
            DistBlockedPct = HardBlockedPct;
        }
        else if (DistBlockedPct > SoftBlockedPct)
        {
            // interpolate smoothly in
            if (PenetrationBlendInTime > DeltaTime)
            {
                DistBlockedPct = DistBlockedPct
                  - DeltaTime / PenetrationBlendInTime
                    * (DistBlockedPct - SoftBlockedPct);
            }
            else
            {
                DistBlockedPct = SoftBlockedPct;
            }
        }
    }

    DistBlockedPct = FMath::Clamp<float>(DistBlockedPct, 0.f, 1.f);
    if (DistBlockedPct < (1.f - ZERO_ANIMWEIGHT_THRESH))
    {
        CameraLoc = SafeLoc + (CameraLoc - SafeLoc) * DistBlockedPct;
    }
}


void UCameraMode::SetBlendWeight(float Weight)
{
    BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

    // Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.
    const float invExponent = (BlendExponent > 0.0f) ? (1.0f / BlendExponent) : 1.0f;

    switch (BlendFunction)
    {
    case ECameraModeBlendFunction::Linear:
        BlendAlpha = BlendWeight;
        break;

    case ECameraModeBlendFunction::EaseIn:
        BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, invExponent);
        break;

    case ECameraModeBlendFunction::EaseOut:
        BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, invExponent);
        break;

    case ECameraModeBlendFunction::EaseInOut:
        BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, invExponent);
        break;

    default:
        UE_LOG(LogTemp, Error, TEXT("UCameraMode: trying to set wrong BlendFunction"));
        break;
    }
}

void UCameraMode::AdjustCameraZoom(float Offset)
{
    if (AllowZoom)
    {
        float newDistance = FMath::Clamp(TargetZoomOffset.X + Offset, CameraDistanceMax, CameraDistanceMin);
        TargetZoomOffset.X = newDistance;
    }
}

void UCameraModeBlender::PushCameraMode(TSubclassOf<UCameraMode> ActiveCameraMode)
{
    if (!ActiveCameraMode)
        return;

    int32 stackSize = CameraModeStack.Num();
    if ((stackSize > 0) && (ActiveCameraMode->GetClass() == CameraModeStack[0]->GetClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("UCameraModeBlender: trying to set the same camera again"));
        return;
    }

    auto CameraMode = GetCameraModeInstance(ActiveCameraMode);
    check(CameraMode);

    // See if it's already in the stack and remove it.
    // Figure out how much it was contributing to the stack.
    int32 existingStackIndex = INDEX_NONE;
    float existingStackContribution = 1.0f;

    for (int32 stackIndex = 0; stackIndex < stackSize; ++stackIndex)
    {
        if (CameraModeStack[stackIndex] == CameraMode)
        {
            existingStackIndex = stackIndex;
            existingStackContribution *= CameraMode->GetBlendWeight();
            break;
        }
        else
        {
            existingStackContribution *= (1.0f - CameraModeStack[stackIndex]->GetBlendWeight());
        }
    }

    if (existingStackIndex != INDEX_NONE)
    {
        CameraModeStack.RemoveAt(existingStackIndex);
        stackSize--;
    }
    else
        existingStackContribution = 0.0f;

    // Decide what initial weight to start with.
    bool shouldBlend = ((CameraMode->GetBlendTime() > 0.0f) && (stackSize > 0));
    float blendWeight = (shouldBlend ? existingStackContribution : 1.0f);

    CameraMode->SetBlendWeight(blendWeight);

    // Add new entry to top of stack.
    CameraModeStack.Insert(CameraMode, 0);

    // Make sure stack bottom is always weighted 100%.
    CameraModeStack.Last()->SetBlendWeight(1.0f);
}

bool UCameraModeBlender::UpdateBlend(float DeltaTime, FCameraModeView& OutCameraView)
{
    int32 stackSize = CameraModeStack.Num();
    if (stackSize <= 0)
        return false;

    int32 removeCount = 0;
    int32 removeIndex = INDEX_NONE;

    for (int32 stackIndex = 0; stackIndex < stackSize; ++stackIndex)
    {
        auto CameraMode = CameraModeStack[stackIndex];
        check(CameraMode);

        CameraMode->UpdateCameraMode(DeltaTime);

        if (CameraMode->GetBlendWeight() >= 1.0f)
        {
            // Everything below this mode is now irrelevant and can be removed.
            removeIndex = (stackIndex + 1);
            removeCount = (stackSize - removeIndex);
            break;
        }
    }

    CameraModeStack.RemoveAt(removeIndex, removeCount);
    stackSize -= removeCount;
    // Start at the bottom and blend up the stack
    OutCameraView = CameraModeStack[stackSize - 1]->GetCameraModeView();

    for (int32 stackIndex = (stackSize - 2); stackIndex >= 0; --stackIndex)
    {
        const auto CameraMode = CameraModeStack[stackIndex];
        check(CameraMode);

        OutCameraView.Blend(CameraMode->GetCameraModeView(), CameraMode->GetBlendWeight());
    }
    return true;
}

UCameraMode *UCameraModeBlender::GetActiveCamera() const 
{
    int32 stackSize = CameraModeStack.Num();
    if (stackSize <= 0)
        return nullptr;
    return CameraModeStack[0];
}

UCameraMode* UCameraModeBlender::GetCameraModeInstance(TSubclassOf<UCameraMode> CameraModeClass)
{
    check(CameraModeClass);

    // First see if we already created one.
    for (UCameraMode* CameraMode : CameraModeInstances)
    {
        if ((CameraMode != nullptr) && (CameraMode->GetClass() == CameraModeClass))
        {
            return CameraMode;
        }
    }

    // Not found, so we need to create it.
    auto NewCameraMode = NewObject<UCameraMode>(GetOuter(), CameraModeClass, NAME_None, RF_NoFlags);
    check(NewCameraMode);
    NewCameraMode->Initialize();
    CameraModeInstances.Add(NewCameraMode);

    return NewCameraMode;
}
