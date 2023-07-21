#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"

#include "PlayerPawnCameraComponent.generated.h"


#define CAMERA_DEFAULT_FOV          (80.0f)
#define CAMERA_DEFAULT_PITCH_MIN    (-89.0f)
#define CAMERA_DEFAULT_PITCH_MAX    (89.0f)


class UCameraMode;
class UCameraModeBlender;
// TODO add camera shake and physics for strikes and getting hit, in editor with anim_notifies and events
/*
 * Class that manages transitions between camera modes and holds info about player's view
 */
UCLASS()
class MADFIRE_API UPlayerPawnCameraComponent : public UCameraComponent
{
    GENERATED_BODY()

public:
    UPlayerPawnCameraComponent(const FObjectInitializer& ObjectInitializer);

    FORCEINLINE AActor* GetTargetActor() const { return GetOwner(); }
    void UpdateCameraMode(TSubclassOf<UCameraMode> CameraMode);
    void AdjustActiveCameraZoom(float Offset);
    TSubclassOf<UCameraMode> GetDefaultCameraMode() const;

    // Activates during sprint only
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UCameraMode> SprintCameraMode;

protected:
    void OnRegister() override;
    void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

    // Default camera mode used by player controlled pawns.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UCameraMode> DefaultCameraModeUnarmed;

    // Default camera mode when melee weapon drawn.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UCameraMode> DefaultCameraModeArmedMelee;

    // Default camera mode used when ranged weapon drawn.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UCameraMode> DefaultCameraModeArmedRange;

private:
    // Used to blend the camera modes.
    UPROPERTY()
    UCameraModeBlender* CameraBlender;
};
