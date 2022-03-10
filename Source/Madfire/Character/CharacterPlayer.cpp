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

void ACharacterPlayer::OnDirectionTimeout(uint8 directions)
{
    CombatComponent->SpecialDirections &= 0xf & ~directions;
    if (directions & (ESpecialDirection::SLEFT | ESpecialDirection::SRIGHT))
        SideGotOld = true;
    else
        FaceGotOld = true;
}

EControlDirections ACharacterPlayer::GetInputDirection()
{
    const auto fwd = InputComponent->GetAxisValue(FInputKeyNames::MoveForward);
    const auto rght = InputComponent->GetAxisValue(FInputKeyNames::MoveRight);
    if (fwd > MovementDeadZone)
    {
        if (rght > MovementDeadZone)
            return EControlDirections::FRightD;
        else if  (rght < -MovementDeadZone)
            return EControlDirections::FLeftD;
        else
            return EControlDirections::ForwardD;
    }
    else if (fwd < -MovementDeadZone)
    {
        if (rght > MovementDeadZone)
            return EControlDirections::BRightD;
        else if  (rght < -MovementDeadZone)
            return EControlDirections::BLeftD;
        else
            return EControlDirections::BackD;
    }
    else if (rght > MovementDeadZone)
    {
        if (fwd > MovementDeadZone)
            return EControlDirections::FRightD;
        else if  (fwd < -MovementDeadZone)
            return EControlDirections::BRightD;
        else
            return EControlDirections::RightD;
    }
    else if (rght < -MovementDeadZone)
    {
        if (fwd > MovementDeadZone)
            return EControlDirections::FLeftD;
        else if  (fwd < -MovementDeadZone)
            return EControlDirections::BLeftD;
        else
            return EControlDirections::LeftD;
    }
    return EControlDirections::Default;
}

bool ACharacterPlayer::IsActionKeyPressed(const FName &ActionKey)
{
    auto playerController = Cast<APlayerController>(Controller);
    TArray<FInputActionKeyMapping> KeyListTest = playerController->PlayerInput->GetKeysForAction(ActionKey);
    if (KeyListTest.Num() == 0)
    {
        UE_LOG(
          LogTemp, Warning, TEXT("No keys set for action %s"), *ActionKey.ToString());
        return false;
    }
    for (const auto &i : KeyListTest)
    {
        if (playerController->IsInputKeyDown(i.Key))
        {
            //UE_LOG(LogTemp, Warning, TEXT("Pressing %s"), *ActionKey.ToString());
            return true;
        }
    }
    return false;
}

void ACharacterPlayer::MoveForward(float AxisValue)
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (FMath::Abs(AxisValue) >= MovementDeadZone && !CCStateManager.HasCC()
                && GetMovementComponent()->CanMove())
            {
                const auto combatState = CombatComponent->CombatFSM.currentState();
                if (combatState != nullptr && combatState->Id == ECombatState::Ability)// check if can move during cast, interrupt then
                {
                    if (CombatComponent->CurrentAbility->MoveAbilities == 0)
                        InterruptAnimation(ECCState::NoCC);
                }
                if (GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Swimming)
                {
                    const FVector Direction =
                      UKismetMathLibrary::GetForwardVector(FRotator(
                        GetControlRotation().Pitch, GetControlRotation().Yaw, 0.f));
                    AddMovementInput(Direction, AxisValue, false);
                }
                else if (GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingLedge:
                        //case ECustomMovementMode::Slide: //deprecated
                        case ECustomMovementMode::Dodge:
                            return;
                        default:
                            return;
                    }
                }
                else
                {
                    const FVector Direction = UKismetMathLibrary::GetForwardVector(FRotator(0.f, GetControlRotation().Yaw, 0.f));
                    //UE_LOG(LogTemp, Warning, TEXT("Forward vector %s"), *Direction.ToString());
                    AddMovementInput(Direction, AxisValue, false);
                    // special attack direction update
                    if (!FaceGotOld)
                    {
                        uint8 direction;
                        if (AxisValue > 0)
                            direction = ESpecialDirection::SFORWARD;
                        else
                            direction = ESpecialDirection::SBACK;
                        if (!(CombatComponent->SpecialDirections & direction))
                        {
                            CombatComponent->SpecialDirections &= 0xf & ~(ESpecialDirection::SFORWARD | ESpecialDirection::SBACK);
                            CombatComponent->SpecialDirections |= direction;
                            GetWorld()->GetTimerManager().SetTimer(FaceTimer,
                                [&, direction]() { OnDirectionTimeout(direction); }, NewDirectionTime, false);
                        }
                    }
                }
            }
            else // reset special attack directions
            {
                CombatComponent->SpecialDirections &= 0xf & ~(ESpecialDirection::SFORWARD | ESpecialDirection::SBACK);
                FaceGotOld = false;
            }
            break;
        }
        default:
            return;
    }
}

void ACharacterPlayer::MoveRight(float AxisValue)
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (FMath::Abs(AxisValue) >= MovementDeadZone && !CCStateManager.HasCC()
                && GetMovementComponent()->CanMove())
            {
                const auto combatState = CombatComponent->CombatFSM.currentState();
                if (combatState != nullptr && combatState->Id == ECombatState::Ability)// check if can move during cast, interrupt then
                {
                    if (CombatComponent->CurrentAbility->MoveAbilities == 0)
                        InterruptAnimation(ECCState::NoCC);
                }
                if (GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingLedge:
                        //case ECustomMovementMode::Slide: // deprecated
                        case ECustomMovementMode::Dodge:
                            return;
                        default:
                            return;
                    }
                }
                else
                {
                    const FRotator Rotation =
                      FRotator(0.f, GetControlRotation().Yaw, 0.f);
                    const FVector Direction =
                      UKismetMathLibrary::GetRightVector(Rotation);
                    AddMovementInput(Direction, AxisValue, false);
                    // special attack direction update
                    if (!SideGotOld)
                    {
                        uint8 direction;
                        if (AxisValue > 0)
                            direction = ESpecialDirection::SRIGHT;
                        else
                            direction = ESpecialDirection::SLEFT;
                        if (!(CombatComponent->SpecialDirections & direction))
                        {
                            CombatComponent->SpecialDirections &= 0xf & ~(ESpecialDirection::SRIGHT | ESpecialDirection::SLEFT);
                            CombatComponent->SpecialDirections |= direction;
                            GetWorld()->GetTimerManager().SetTimer(SideTimer,
                                [&, direction]() { OnDirectionTimeout(direction); }, NewDirectionTime, false);
                        }
                    }
                    //if (GetWorld()->GetTimerManager().IsTimerActive(SideTimer))
                    //GetWorld()->GetTimerManager().ClearTimer(SideTimer);
                }
                if (GetMovementComponent()->bWantToSprint && 
                    CombatComponent->WeaponDrawn() && 
                    InputComponent->GetAxisValue(FInputKeyNames::MoveForward) <= 0.f)
                    GetMovementComponent()->bWantToSprint = false;
            }
            else // reset special attack directions
            {
                CombatComponent->SpecialDirections &= 0xf & ~(ESpecialDirection::SLEFT | ESpecialDirection::SRIGHT);
                SideGotOld = false;
            }
        }
        default:
            return;
    }
}

void ACharacterPlayer::CameraPitch(float AxisValue)
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld):
        case (EPawnGameplayState::InMountMode): {
            auto playerController = Cast<APlayerController>(Controller);
            playerController->AddPitchInput(AxisValue * GetWorld()->GetDeltaSeconds());
        }
        default:
            return;
    }
}

void ACharacterPlayer::CameraYaw(float AxisValue)
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld):
        case (EPawnGameplayState::InMountMode): {
            auto playerController = Cast<APlayerController>(Controller);
            playerController->AddYawInput(AxisValue * GetWorld()->GetDeltaSeconds());
        }
        default:
            return;
    }
}

void ACharacterPlayer::RunAction()
{
    switch (CurrentGameplayState)
    {
        case (EPawnGameplayState::InWorld):    //todo учесть другие состояния
        {
            if (GetMovementComponent()->bIsRunning)
                GetMovementComponent()->bIsRunning = false;
            else
                GetMovementComponent()->bIsRunning = true;
        }
        default:
            return;
    }
}

void ACharacterPlayer::SprintAction()    // TODO add equipment weight check
{
    switch (CurrentGameplayState)
    {
        case (EPawnGameplayState::InWorld):    //todo учесть другие состояния
        {
            if (GetMovementComponent()->IsMovingOnGround()
                && !CCStateManager.HasCC())
            {
                if ((CombatComponent->WeaponDrawn()
                    && InputComponent->GetAxisValue(FInputKeyNames::MoveForward) <= 0.f) ||
                    CombatComponent->IsBusy())
                    return;
                GetMovementComponent()->bWantToSprint = true;
                if (bIsCrouched)
                    UnCrouch();
            }
        }
        default:
            return;
    }
}

void ACharacterPlayer::UnSprintAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (GetMovementComponent()->bWantToSprint)
            {
                GetMovementComponent()->bWantToSprint = false;
            }
        }
        default:
            return;
    }
}

void ACharacterPlayer::CrouchAction()
{
    GetMovementComponent()->SetMovementMode(
      EMovementMode::MOVE_Walking);    // todo temp
    switch (CurrentGameplayState)
    {
        case (EPawnGameplayState::InWorld):    //todo учесть другие состояния
        {
            if (!CCStateManager.HasCC()
                && !GetMovementComponent()->IsRootOrTeleport())
            {
                if (GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Walking)
                {
                    if (CombatComponent->IsBusy())
                        return;
                    if (GetMovementComponent()->IsCrouching())
                        UnCrouch();
                    else
                        Crouch();
                }
                else if (GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingWall: {
                            GetMovementComponent()->GetOffWall();
                            break;
                        }
                        case ECustomMovementMode::ClimbingLedge:
                        //case ECustomMovementMode::ClimbingLadder: // deprecated
                        case ECustomMovementMode::Dodge:
                        //case ECustomMovementMode::Slide: // deprecated
                        default:
                            break;
                    }
                }
            }
            break;
        }
        default:
            return;
    }
}

void ACharacterPlayer::JumpAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!CCStateManager.HasCC()
                && GetMovementComponent()->CanMove())
            {
                if (GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Walking)
                {
                    if (InputComponent->GetAxisValue(FInputKeyNames::MoveForward) != 0.f
                        || InputComponent->GetAxisValue(FInputKeyNames::MoveRight) != 0.f)
                    {
                        if (GetMovementComponent()->CanVaultOrClimb(true))
                            return;
                        else if (GetMovementComponent()->CanVaultOrClimb(false))
                            return;
                    }
                    int currentCombatState = CombatComponent->CombatFSM.currentState()->Id;
                    bool canInterruptAction = (currentCombatState == ECombatState::Attack || currentCombatState == ECombatState::Parry) &&
                        !CombatComponent->CombatFSM.currentState()->Info.CheckAnimInfo(ECombatStateAnimInfo::MovementBlocked) &&
                        !CombatComponent->CombatFSM.currentState()->Info.CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed);
                    if (currentCombatState == ECombatState::Block || canInterruptAction)
                        InterruptAnimation(ECCState::NoCC);
                    Jump();
                    return;
                }
                else if (GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingLedge:
                            break;
                        case ECustomMovementMode::ClimbingWall: {
                            if (InputComponent->GetAxisValue(FInputKeyNames::MoveForward)
                                > 0.f)
                                GetMovementComponent()->ClimbWall();
                            else
                            {
                                const auto fwd = InputComponent->GetAxisValue(
                                  FInputKeyNames::MoveForward);
                                const auto rght =
                                  InputComponent->GetAxisValue(FInputKeyNames::MoveRight);
                                if (fwd != 0 || rght != 0)
                                {
                                    if (FMath::Abs(fwd) > FMath::Abs(rght))
                                        GetMovementComponent()->JumpOffWall(
                                          EControlDirections::BackD);
                                    else if (rght > 0)
                                        GetMovementComponent()->JumpOffWall(
                                          EControlDirections::RightD);
                                    else
                                        GetMovementComponent()->JumpOffWall(
                                          EControlDirections::LeftD);
                                }
                            }
                            break;
                        }
                        //case ECustomMovementMode::ClimbingLadder: // deprecated
                        case ECustomMovementMode::Dodge:
                        //case ECustomMovementMode::Slide: // deprecated
                        default:
                            break;
                    }
                }
                else if (GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Falling)
                {
                    if (GetMovementComponent()->CheckCanHangAgain())
                        GetMovementComponent()->HangOnFall();
                }
            }
            break;
        }
        default:
            return;
    }
}
void ACharacterPlayer::UnJumpAction() { StopJumping(); }

void ACharacterPlayer::JumpRepAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!CCStateManager.HasCC()
                && !GetMovementComponent()->CanMove())
            {
                if (GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Falling)
                {
                    if (GetMovementComponent()->CheckCanHangAgain())
                        GetMovementComponent()->HangOnFall();
                }
            }
            break;
        }
    }
}

void ACharacterPlayer::DodgeAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!CCStateManager.HasCC()
                && GetMovementComponent()->CanMove() && CombatComponent->WeaponDrawn())
            {
                if (GetMovementComponent()->MovementMode == EMovementMode::MOVE_Walking)
                    GetMovementComponent()->DoDodge(GetInputDirection());
            }
            break;
        }
    }
}

void ACharacterPlayer::AttackOnAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
    case (EPawnGameplayState::InWorld): {
        if (!CCStateManager.HasCC())
            CombatComponent->OnAttack(true); // TODO check if ability is busy
    }
    }
}

void ACharacterPlayer::AttackOffAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!CCStateManager.HasCC())
                CombatComponent->OnAttack(false);
        }
    }
}

void ACharacterPlayer::BlockOnAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!GetMovementComponent()->IsCrouching() &&
                GetMovementComponent()->MovementMode == EMovementMode::MOVE_Walking)
            {
                if (!CCStateManager.HasCC() ||
                    (CCStateManager.IsInCCMode(ECCState::LossOfBalance) && CCStateManager.IsInCCSubMode(BlockedAttack)))
                    CombatComponent->OnBlock(true);
            }
        }
    }
}

void ACharacterPlayer::BlockOffAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!GetMovementComponent()->IsCrouching())
            {
                if (!CCStateManager.HasCC() ||
                    (CCStateManager.IsInCCMode(ECCState::LossOfBalance) && CCStateManager.IsInCCSubMode(BlockedAttack)))
                    CombatComponent->OnBlock(false);
            }
        }
    }
}

void ACharacterPlayer::WeaponAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!CCStateManager.HasCC() && GetMovementComponent()->MovementMode != EMovementMode::MOVE_Custom)
                    CombatComponent->OnDrawWeapon();
        }
    }
}

void ACharacterPlayer::AbilityOn()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!CCStateManager.HasCC())
                CombatComponent->OnUseAbility(true);
        }
    }
}

void ACharacterPlayer::AilityOff()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!CCStateManager.HasCC())
                CombatComponent->OnUseAbility(false);
        }
    }
}

void ACharacterPlayer::CameraDistanceIn()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            float Length = SpringArmComponent->TargetArmLength - CameraZoomStep;
            SpringArmComponent->TargetArmLength = FMath::Max(Length, CameraDistanceMin);
        }
    }
}

void ACharacterPlayer::CameraDistanceOut()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            float Length = SpringArmComponent->TargetArmLength + CameraZoomStep;
            SpringArmComponent->TargetArmLength = FMath::Min(Length, CameraDistanceMax);
        }
    }
}

void ACharacterPlayer::InteractAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            OnInteract();
        }
    }
}
