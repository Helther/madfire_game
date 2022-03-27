#include "CharacterPlayer.h"

#include "PlayerCharController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "AbilitySystem/AbilityComponents/AbilityAimComponent.h"

ACharacterPlayer::ACharacterPlayer(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->SetupAttachment(RootComponent);
    CameraComponent->SetupAttachment(SpringArmComponent);

    CameraDistanceDefault = 400;
    CameraDistanceMin = 100;
    CameraDistanceMax = 500;
    CameraZoomStep = 30;
    SpringArmComponent->TargetArmLength = CameraDistanceDefault;
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

void ACharacterPlayer::OnWalkingOffLedge_Implementation(
  const FVector &PreviousFloorImpactNormal,
  const FVector &PreviousFloorContactNormal,
  const FVector &PreviousLocation,
  float TimeDelta)
{
    Logger.Log(this,*FString::Printf(TEXT("OnWalkingOffLedge")));
    APlayerCharController *PlayerController =
      Cast<APlayerCharController>(GetController());
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

