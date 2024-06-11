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
    GetССComponent()->OnCCStateChanged.AddDynamic(this, &ACharacterPlayer::ApplyCameraShakeForCCState);
    GetCombatComponent()->OnCombatStateChanged.AddDynamic(this, &ACharacterPlayer::UpdateDefaultCameraMode);
    CameraZoomStep = 70;
    LossOfBalanceShakeScale = 1.f;
    StunShakeScale = 1.5f;
    KnockDownShakeScale = 2.f;
    BlockedHitShakeScale = 0.8f;
    RangedWeaponFireShakeScaleMax = 1.f;
    RangedWeaponTensionMax = 3000.f;
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
    CameraComponent->UpdateCameraMode(CameraComponent->GetDefaultCameraMode());
}

void ACharacterPlayer::SetSprintCameraMode(bool activate)
{
    CameraComponent->UpdateCameraMode(activate ? CameraComponent->SprintCameraMode : CameraComponent->GetDefaultCameraMode());
}

void ACharacterPlayer::AdjustCameraZoom(float Offset) 
{
    CameraComponent->AdjustActiveCameraZoom(Offset);
}
/*
void ACharacterPlayer::FireLoadedProjectile()
{
    ACharacterCreature::FireLoadedProjectile();

    if (auto RangedWeapon = GetCombatComponent()->GetEquippedRangedWeapon())
    {  // ranged attack
        if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
        {
            float scale = FMath::GetMappedRangeValueClamped(FVector2D(0.f, RangedWeaponTensionMax), FVector2D(0.f, RangedWeaponFireShakeScaleMax),
                RangedWeapon->MaxTensionStrength);
            PlayerController->PlayerCameraManager->StartCameraShake(FireCameraShake, scale);
        }
    }
}*/

void ACharacterPlayer::OnWalkingOffLedge_Implementation(const FVector &PreviousFloorImpactNormal, const FVector &PreviousFloorContactNormal,const FVector &PreviousLocation, float TimeDelta)
{

    APlayerCharController *PlayerController = Cast<APlayerCharController>(GetController());
    if (PlayerController->IsActionKeyPressed(FInputKeyNames::ClimbDownTest))
    {
        GetMovementComponent()->ClimbDownWall();
    }
}

void ACharacterPlayer::ApplyCameraShakeForCCState(unsigned NewState, unsigned NewSubState)
{
    APlayerController *PlayerController = Cast<APlayerController>(GetController());
    if (!PlayerController)
        return;

    float scale;
    switch (NewState)
    {
        case ECCState::LossOfBalance:
            //if (NewSubState == ELossOfBalanceCCSubState::BlockedAttack)
            if (NewSubState == static_cast<int>(ELossOfBalanceCCSubState::BlockedAttack))
                scale = BlockedHitShakeScale;
            else
                scale = LossOfBalanceShakeScale;
            break;
        case ECCState::Stun:
            scale = StunShakeScale;
            break;
        case ECCState::KnockDown:
            scale = KnockDownShakeScale;
            break;
        default:
            return;
    }
    PlayerController->PlayerCameraManager->StartCameraShake(CCCameraShake, scale);
}

void ACharacterPlayer::UpdateDefaultCameraMode(int OldState, int NewState)
{
    if(NewState == ECombatState::Idle)  // Switches cam after weapon switch, if there was one, otherwise same cam remains
        CameraComponent->UpdateCameraMode(CameraComponent->GetDefaultCameraMode());
}

void ACharacterPlayer::UseAimComponent(UAbilityAimComponent *AimComp)
{
    if(!AimComp) 
        return;

    AimComp->ActivateComponent();
}
