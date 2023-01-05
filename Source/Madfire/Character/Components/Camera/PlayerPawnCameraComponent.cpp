#include "PlayerPawnCameraComponent.h"
#include "CameraMode.h"


UPlayerPawnCameraComponent::UPlayerPawnCameraComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CameraBlender = nullptr;
}

void UPlayerPawnCameraComponent::UpdateCameraMode(TSubclassOf<UCameraMode> CameraMode)
{
    CameraBlender->PushCameraMode(CameraMode);
}

void UPlayerPawnCameraComponent::AdjustActiveCameraZoom(float Offset) 
{
    if (auto camera = CameraBlender->GetActiveCamera())
        camera->AdjustCameraZoom(Offset);
}

void UPlayerPawnCameraComponent::OnRegister()
{
    Super::OnRegister();

    if (!CameraBlender)
    {
        CameraBlender = NewObject<UCameraModeBlender>(this);
        check(CameraBlender);
    }
}

void UPlayerPawnCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
    FCameraModeView CameraModeView;
    CameraBlender->UpdateBlend(DeltaTime, CameraModeView);
    if (bUsePawnControlRotation)
    {
        if (APawn *TargetPawn = Cast<APawn>(GetTargetActor()))
        {
            if (APlayerController *PC = TargetPawn->GetController<APlayerController>())
                PC->SetControlRotation(CameraModeView.ControlRotation);
        }
    }
    // Keep camera component in sync with the latest view.
    //SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
    FieldOfView = CameraModeView.FieldOfView;

    // Fill in desired view.
    DesiredView.Location = CameraModeView.Location;
    DesiredView.Rotation = CameraModeView.Rotation;
    DesiredView.FOV = CameraModeView.FieldOfView;
    DesiredView.OrthoWidth = OrthoWidth;
    DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
    DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
    DesiredView.AspectRatio = AspectRatio;
    DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
    DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
    DesiredView.ProjectionMode = ProjectionMode;

    // See if the CameraActor wants to override the PostProcess settings used.
    DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
    if (PostProcessBlendWeight > 0.0f)
    {
        DesiredView.PostProcessSettings = PostProcessSettings;
    }
}
