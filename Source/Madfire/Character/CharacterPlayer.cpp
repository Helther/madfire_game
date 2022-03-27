#include "CharacterPlayer.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/KismetMathLibrary.h"

struct FInputKeyNames
{
    static const FName MoveForward;
    static const FName MoveRight;
    static const FName Run;
    static const FName Sprint;
    static const FName Crouch;
    static const FName Jump;
    static const FName Dodge;
    static const FName CameraPitch;
    static const FName CameraYaw;
    static const FName Attack;
    static const FName Block;
    static const FName DrawWeapon;
    static const FName UseAbility;
    static const FName ClimbDownTest;
    static const FName CameraIn;
    static const FName CameraOut;
    static const FName Interact;
};

const FName FInputKeyNames::MoveForward   = "MoveForward";
const FName FInputKeyNames::MoveRight     = "MoveRight";
const FName FInputKeyNames::Run           = "Run";
const FName FInputKeyNames::Sprint        = "Sprint";
const FName FInputKeyNames::Crouch        = "Crouch";
const FName FInputKeyNames::Jump          = "Jump";
const FName FInputKeyNames::Dodge         = "Dodge";
const FName FInputKeyNames::CameraPitch   = "CameraPitch";
const FName FInputKeyNames::CameraYaw     = "CameraYaw";
const FName FInputKeyNames::Attack        = "Attack";
const FName FInputKeyNames::Block         = "Block";
const FName FInputKeyNames::DrawWeapon    = "Draw weapon";
const FName FInputKeyNames::UseAbility    = "UseAbility";
const FName FInputKeyNames::ClimbDownTest = "ClimbDownTest";
const FName FInputKeyNames::CameraIn      = "Camera zoom in";
const FName FInputKeyNames::CameraOut     = "Camera zoom out";
const FName FInputKeyNames::Interact      = "Interact";


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

    NewDirectionTime = 0.5f;
    MovementDeadZone = 0.3f;
}

void ACharacterPlayer::BeginPlay()
{
    Super::BeginPlay();
    CurrentGameplayState = EPawnGameplayState::InWorld;    // debug
}

void ACharacterPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!Controller || !Controller->InputComponent)
        return;
    if (CurrentGameplayState == EPawnGameplayState::InWorld
        && IsActionKeyPressed(FInputKeyNames::Jump))
        JumpRepAction();
}

// Called to bind functionality to input
void ACharacterPlayer::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    InputComponent->BindAction(
        FInputKeyNames::Run, IE_Pressed, this, &ACharacterPlayer::RunAction);
    InputComponent->BindAction(
        FInputKeyNames::Sprint, IE_Pressed, this, &ACharacterPlayer::SprintAction);
    InputComponent->BindAction(FInputKeyNames::Sprint,
                                IE_Released,
                                this,
                                &ACharacterPlayer::UnSprintAction);
    InputComponent->BindAction(
        FInputKeyNames::Crouch, IE_Pressed, this, &ACharacterPlayer::CrouchAction);
    InputComponent->BindAction(
        FInputKeyNames::Jump, IE_Pressed, this, &ACharacterPlayer::JumpAction);
    InputComponent->BindAction(
        FInputKeyNames::Jump, IE_Released, this, &ACharacterPlayer::UnJumpAction);
    InputComponent->BindAction(
        FInputKeyNames::Dodge, IE_Pressed, this, &ACharacterPlayer::DodgeAction);
    InputComponent->BindAxis(
        FInputKeyNames::MoveForward, this, &ACharacterPlayer::MoveForward);
    InputComponent->BindAxis(
        FInputKeyNames::MoveRight, this, &ACharacterPlayer::MoveRight);
    InputComponent->BindAxis(
        FInputKeyNames::CameraPitch, this, &ACharacterPlayer::CameraPitch);
    InputComponent->BindAxis(
        FInputKeyNames::CameraYaw, this, &ACharacterPlayer::CameraYaw);
    InputComponent->BindAction(
        FInputKeyNames::Attack, IE_Pressed, this, &ACharacterPlayer::AttackOnAction);
    InputComponent->BindAction(
        FInputKeyNames::Attack, IE_Released, this, &ACharacterPlayer::AttackOffAction);
    InputComponent->BindAction(
        FInputKeyNames::Block, IE_Pressed, this, &ACharacterPlayer::BlockOnAction);
    InputComponent->BindAction(
        FInputKeyNames::Block, IE_Released, this, &ACharacterPlayer::BlockOffAction);
    InputComponent->BindAction(
        FInputKeyNames::DrawWeapon, IE_Pressed, this, &ACharacterPlayer::WeaponAction);
    InputComponent->BindAction(
        FInputKeyNames::UseAbility, IE_Pressed, this, &ACharacterPlayer::AbilityOn);
    InputComponent->BindAction(
        FInputKeyNames::UseAbility, IE_Released, this, &ACharacterPlayer::AilityOff);
    InputComponent->BindAction(
        FInputKeyNames::CameraIn, IE_Pressed, this, &ACharacterPlayer::CameraDistanceIn);
    InputComponent->BindAction(
        FInputKeyNames::CameraOut, IE_Pressed, this, &ACharacterPlayer::CameraDistanceOut);
    InputComponent->BindAction(
        FInputKeyNames::Interact, IE_Pressed, this, &ACharacterPlayer::InteractAction);
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
    if (IsActionKeyPressed(FInputKeyNames::ClimbDownTest))
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

