#include "CharacterPlayer.h"

#include "PlayerCharController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "AbilitySystem/Components/AbilityAimComponent.h"
#include "Components/Camera/PlayerPawnCameraComponent.h"


ACharacterPlayer::ACharacterPlayer(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    CameraComponent = CreateDefaultSubobject<UPlayerPawnCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));
    GetMovementComponent()->OnSprintModeChanged.AddDynamic(this, &ACharacterPlayer::SetSprintCameraMode);

    CameraZoomStep = 30;
}

void ACharacterPlayer::BeginPlay()
{
    Super::BeginPlay();

    ClearAbilityCameraMode();
}

// Called to bind functionality to input
void ACharacterPlayer::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACharacterPlayer::OnJumped_Implementation()
{
    GetMovementComponent()->bCanHangAgain   = false;
    GetMovementComponent()->bWasJustJumping = true;
}

void ACharacterPlayer::SetAbilityCameraMode(TSubclassOf<UCameraMode> CameraMode)
{
    CameraComponent->UpdateCameraMode(CameraMode);
}

void ACharacterPlayer::ClearAbilityCameraMode()
{
    CameraComponent->UpdateCameraMode(CameraComponent->DefaultCameraMode);
}

void ACharacterPlayer::SetSprintCameraMode(bool activate)
{
    CameraComponent->UpdateCameraMode(activate ? CameraComponent->SprintCameraMode
                                               : CameraComponent->DefaultCameraMode);
}

void ACharacterPlayer::AdjustCameraZoom(float Offset) 
{
    CameraComponent->AdjustActiveCameraZoom(Offset);
}

void ACharacterPlayer::OnWalkingOffLedge_Implementation(
  const FVector &PreviousFloorImpactNormal,
  const FVector &PreviousFloorContactNormal,
  const FVector &PreviousLocation,
  float TimeDelta)
{
    Logger.Log(this,*FString::Printf(TEXT("OnWalkingOffLedge")));
    APlayerCharController *PlayerController = Cast<APlayerCharController>(GetController());
    if (PlayerController->IsActionKeyPressed(FInputKeyNames::ClimbDownTest))
    {
        Logger.Log(this,*FString::Printf(TEXT("climb down")));
        GetMovementComponent()->ClimbDownWall();
    }
}

void ACharacterPlayer::UseAimComponent(UAbilityAimComponent *AimComp)
{
    if(!AimComp) return;

    AimComp->ActivateComponent();
}

