#include "PlayerCharController.h"
// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharController.h"


#include "GameFramework/PlayerInput.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/CharacterPlayer.h"
#include "WidgetTree.h"

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
const FName FInputKeyNames::OpenMainMenu  = "OpenMainMenu";
const FName FInputKeyNames::OpenInventoryMenu = "OpenInventoryMenu";
const FName FInputKeyNames::OpenAbilityManager = "OpenAbilityManager";
const FName FInputKeyNames::OpenCharacterMenu = "OpenCharacterMenu";


APlayerCharController::APlayerCharController() 
{
    // debug
    NewDirectionTime = 0.5f;
    MovementDeadZone = 0.3f;
}

EControlDirections APlayerCharController::GetInputDirection()
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

void APlayerCharController::BeginPlay()
{
    Super::BeginPlay();
    PlayerRef = Cast<ACharacterPlayer>(GetPawn());
    check(Player != nullptr && "APlayerCharController: No pawn");
    CurrentGameplayState = EPawnGameplayState::InWorld;    // debug
    PlayerWidgetTree     = NewObject<UWidgetTree>(this);
}

void APlayerCharController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (CurrentGameplayState == EPawnGameplayState::InWorld
        && IsActionKeyPressed(FInputKeyNames::Jump))
        JumpRepAction();
}

//////////////======================Input========//////////////////////

void APlayerCharController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (InputComponent)
    {
        InputComponent->BindAction(
          FInputKeyNames::Run, IE_Pressed, this, &APlayerCharController::RunAction).bConsumeInput = false;
        InputComponent->BindAction(
          FInputKeyNames::Sprint, IE_Pressed, this, &APlayerCharController::SprintAction).bConsumeInput = false;
        InputComponent->BindAction(FInputKeyNames::Sprint, IE_Released, this, &APlayerCharController::UnSprintAction).bConsumeInput = false;
        InputComponent->BindAction(
          FInputKeyNames::Crouch, IE_Pressed, this, &APlayerCharController::CrouchAction).bConsumeInput = false;
        InputComponent->BindAction(
          FInputKeyNames::Jump, IE_Pressed, this, &APlayerCharController::JumpAction).bConsumeInput = false;
        InputComponent->BindAction(
          FInputKeyNames::Jump, IE_Released, this, &APlayerCharController::UnJumpAction).bConsumeInput = false;
        InputComponent->BindAction(
          FInputKeyNames::Dodge, IE_Pressed, this, &APlayerCharController::DodgeAction).bConsumeInput = false;
        InputComponent->BindAxis(
          FInputKeyNames::MoveForward, this, &APlayerCharController::MoveForward).bConsumeInput = false;
        InputComponent->BindAxis(
          FInputKeyNames::MoveRight, this, &APlayerCharController::MoveRight).bConsumeInput = false;
        InputComponent->BindAxis(
          FInputKeyNames::CameraPitch, this, &APlayerCharController::CameraPitch).bConsumeInput = false;
        InputComponent->BindAxis(
          FInputKeyNames::CameraYaw, this, &APlayerCharController::CameraYaw).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::Attack, IE_Pressed, this, &APlayerCharController::AttackOnAction).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::Attack, IE_Released, this, &APlayerCharController::AttackOffAction).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::Block, IE_Pressed, this, &APlayerCharController::BlockOnAction).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::Block, IE_Released, this, &APlayerCharController::BlockOffAction).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::DrawWeapon, IE_Pressed, this, &APlayerCharController::WeaponAction).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::UseAbility, IE_Pressed, this, &APlayerCharController::AbilityOn).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::UseAbility, IE_Released, this, &APlayerCharController::AbilityOff).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::CameraIn, IE_Pressed, this, &APlayerCharController::CameraDistanceIn).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::CameraOut, IE_Pressed, this, &APlayerCharController::CameraDistanceOut).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::Interact, IE_Pressed, this, &APlayerCharController::InteractAction).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::OpenMainMenu, IE_Pressed, this, &APlayerCharController::OpenMainMenu).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::OpenCharacterMenu, IE_Pressed, this, &APlayerCharController::OpenCharacterMenu).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::OpenInventoryMenu, IE_Pressed, this, &APlayerCharController::OpenInventory).bConsumeInput = false;
        InputComponent->BindAction(
            FInputKeyNames::OpenAbilityManager, IE_Pressed, this, &APlayerCharController::OpenAbilityManager).bConsumeInput = false;

    }
}

bool APlayerCharController::IsActionKeyPressed(const FName &ActionKey)
{
    TArray<FInputActionKeyMapping> KeyListTest = PlayerInput->GetKeysForAction(ActionKey);
    if (KeyListTest.Num() == 0)
    {
        UE_LOG(
          LogTemp, Warning, TEXT("No keys set for action %s"), *ActionKey.ToString());
        return false;
    }
    for (const auto &i : KeyListTest)
    {
        if (IsInputKeyDown(i.Key))
        {
            //UE_LOG(LogTemp, Warning, TEXT("Pressing %s"), *ActionKey.ToString());
            return true;
        }
    }
    return false;
}

void APlayerCharController::OnDirectionTimeout(uint8 directions)
{
    PlayerRef->GetCombatComponent()->SpecialDirections &= 0xf & ~directions;
    if (directions & (ESpecialDirection::SLEFT | ESpecialDirection::SRIGHT))
        SideGotOld = true;
    else
        FaceGotOld = true;
}

void APlayerCharController::MoveForward(float AxisValue)
{
    check(PlayerRef != nullptr && "APlayerCharController: No pawn");
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (FMath::Abs(AxisValue) >= MovementDeadZone && !PlayerRef->GetССComponent()->HasCC()
                && PlayerRef->GetMovementComponent()->CanMove())
            {
                if (PlayerRef->GetCombatComponent()->HasCombatStance()
                    && PlayerRef->GetCombatComponent()->GetCombatFSM().isInState(
                      ECombatState::
                        Ability)) // check if can move during cast, interrupt then
                {
                    if (PlayerRef->GetCombatComponent()->CurrentAbility->MoveAbilities == 0)
                        PlayerRef->InterruptAnimation(ECCState::NoCC);
                }
                if (PlayerRef->GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Swimming)
                {
                    const FVector Direction =
                      UKismetMathLibrary::GetForwardVector(FRotator(
                        GetControlRotation().Pitch, GetControlRotation().Yaw, 0.f));
                    PlayerRef->AddMovementInput(Direction, AxisValue, false);
                }
                else if (PlayerRef->GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (PlayerRef->GetMovementComponent()->CustomMovementMode)
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
                    PlayerRef->AddMovementInput(Direction, AxisValue, false);
                    // special attack direction update
                    if (!FaceGotOld)
                    {
                        uint8 direction;
                        if (AxisValue > 0)
                            direction = ESpecialDirection::SFORWARD;
                        else
                            direction = ESpecialDirection::SBACK;
                        if (!(PlayerRef->GetCombatComponent()->SpecialDirections & direction))
                        {
                            PlayerRef->GetCombatComponent()->SpecialDirections &= 0xf & ~(ESpecialDirection::SFORWARD | ESpecialDirection::SBACK);
                            PlayerRef->GetCombatComponent()->SpecialDirections |= direction;
                            GetWorld()->GetTimerManager().SetTimer(FaceTimer,
                                [&, direction]() { OnDirectionTimeout(direction); }, NewDirectionTime, false);
                        }
                    }
                }
            }
            else // reset special attack directions
            {
                PlayerRef->GetCombatComponent()->SpecialDirections &= 0xf & ~(ESpecialDirection::SFORWARD | ESpecialDirection::SBACK);
                FaceGotOld = false;
            }
            break;
        }
        default:
            return;
    }
}

void APlayerCharController::MoveRight(float AxisValue)
{
    check(PlayerRef != nullptr && "APlayerCharController: No pawn");
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (FMath::Abs(AxisValue) >= MovementDeadZone && !PlayerRef->GetССComponent()->HasCC()
                && PlayerRef->GetMovementComponent()->CanMove())
            {
                if (PlayerRef->GetCombatComponent()->HasCombatStance() && PlayerRef->GetCombatComponent()->GetCombatFSM().isInState(ECombatState::Ability))// check if can move during cast, interrupt then
                {
                    if (PlayerRef->GetCombatComponent()->CurrentAbility->MoveAbilities == 0)
                        PlayerRef->InterruptAnimation(ECCState::NoCC);
                }
                if (PlayerRef->GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (PlayerRef->GetMovementComponent()->CustomMovementMode)
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
                      FRotator(0.f, PlayerRef->GetControlRotation().Yaw, 0.f);
                    const FVector Direction =
                      UKismetMathLibrary::GetRightVector(Rotation);
                    PlayerRef->AddMovementInput(Direction, AxisValue, false);
                    // special attack direction update
                    if (!SideGotOld)
                    {
                        uint8 direction;
                        if (AxisValue > 0)
                            direction = ESpecialDirection::SRIGHT;
                        else
                            direction = ESpecialDirection::SLEFT;
                        if (!(PlayerRef->GetCombatComponent()->SpecialDirections & direction))
                        {
                            PlayerRef->GetCombatComponent()->SpecialDirections &= 0xf & ~(ESpecialDirection::SRIGHT | ESpecialDirection::SLEFT);
                            PlayerRef->GetCombatComponent()->SpecialDirections |= direction;
                            GetWorld()->GetTimerManager().SetTimer(SideTimer,
                                [&, direction]() { OnDirectionTimeout(direction); }, NewDirectionTime, false);
                        }
                    }
                    //if (GetWorld()->GetTimerManager().IsTimerActive(SideTimer))
                    //GetWorld()->GetTimerManager().ClearTimer(SideTimer);
                }
                if (PlayerRef->GetMovementComponent()->bWantToSprint && 
                    PlayerRef->GetCombatComponent()->WeaponDrawn() && 
                    InputComponent->GetAxisValue(FInputKeyNames::MoveForward) <= 0.f)
                    PlayerRef->GetMovementComponent()->bWantToSprint = false;
            }
            else // reset special attack directions
            {
                PlayerRef->GetCombatComponent()->SpecialDirections &= 0xf & ~(ESpecialDirection::SLEFT | ESpecialDirection::SRIGHT);
                SideGotOld = false;
            }
        }
        default:
            return;
    }
}

void APlayerCharController::CameraPitch(float AxisValue)
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld):
        case (EPawnGameplayState::InMountMode): {
            AddPitchInput(AxisValue * GetWorld()->GetDeltaSeconds());
            if (!FMath::IsNearlyEqual(AxisValue, 0, KINDA_SMALL_NUMBER))
                PlayerRef->GetCombatComponent()->AddRangedWeaponSpread();
        }
        default:
            return;
    }
}

void APlayerCharController::CameraYaw(float AxisValue)
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld):
        case (EPawnGameplayState::InMountMode): {
            AddYawInput(AxisValue * GetWorld()->GetDeltaSeconds());
            if (!FMath::IsNearlyEqual(AxisValue, 0, KINDA_SMALL_NUMBER))
                PlayerRef->GetCombatComponent()->AddRangedWeaponSpread();
        }
        default:
            return;
    }
}

void APlayerCharController::RunAction()
{
    check(PlayerRef != nullptr && "APlayerCharController: No pawn");

    switch (CurrentGameplayState)
    {
        case (EPawnGameplayState::InWorld):    //todo учесть другие состояния
        {
            if (PlayerRef->GetMovementComponent()->bIsRunning)
                PlayerRef->GetMovementComponent()->bIsRunning = false;
            else
                PlayerRef->GetMovementComponent()->bIsRunning = true;
        }
        default:
            return;
    }
}

void APlayerCharController::SprintAction()    // TODO add equipment weight check
{
    check(PlayerRef != nullptr && "APlayerCharController: No pawn");

    switch (CurrentGameplayState)
    {
        case (EPawnGameplayState::InWorld):    //todo учесть другие состояния
        {
            if (PlayerRef->GetMovementComponent()->IsMovingOnGround()
                && !PlayerRef->GetССComponent()->HasCC())
            {
                if ((PlayerRef->GetCombatComponent()->WeaponDrawn()
                    && InputComponent->GetAxisValue(FInputKeyNames::MoveForward) <= 0.f) ||
                    PlayerRef->GetCombatComponent()->IsBusy())
                    return;
                PlayerRef->GetMovementComponent()->bWantToSprint = true;
                if (PlayerRef->bIsCrouched)
                    PlayerRef->UnCrouch();
            }
        }
        default:
            return;
    }
}

void APlayerCharController::UnSprintAction()
{
    check(PlayerRef != nullptr && "APlayerCharController: No pawn");

    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (PlayerRef->GetMovementComponent()->bWantToSprint)
            {
                PlayerRef->GetMovementComponent()->bWantToSprint = false;
            }
        }
        default:
            return;
    }
}

void APlayerCharController::CrouchAction()
{
    check(PlayerRef != nullptr && "APlayerCharController: No pawn");
    PlayerRef->GetMovementComponent()->SetMovementMode(
      EMovementMode::MOVE_Walking);    // todo temp
    switch (CurrentGameplayState)
    {
        case (EPawnGameplayState::InWorld):    //todo учесть другие состояния
        {
            if (!PlayerRef->GetССComponent()->HasCC()
                && !PlayerRef->GetMovementComponent()->IsRootOrTeleport())
            {
                if (PlayerRef->GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Walking)
                {
                    if (PlayerRef->GetCombatComponent()->IsBusy())
                        return;
                    if (PlayerRef->GetMovementComponent()->IsCrouching())
                        PlayerRef->UnCrouch();
                    else
                        PlayerRef->Crouch();
                }
                else if (PlayerRef->GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (PlayerRef->GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingWall: {
                            PlayerRef->GetMovementComponent()->GetOffWall();
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

void APlayerCharController::JumpAction()
{
    check(PlayerRef != nullptr && "APlayerCharController: No pawn");

    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!PlayerRef->GetССComponent()->HasCC()
                && PlayerRef->GetMovementComponent()->CanMove())
            {
                if (PlayerRef->GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Walking)
                {
                    if (InputComponent->GetAxisValue(FInputKeyNames::MoveForward) != 0.f
                        || InputComponent->GetAxisValue(FInputKeyNames::MoveRight) != 0.f)
                    {
                        if (PlayerRef->GetMovementComponent()->CanVaultOrClimb(true))
                            return;
                        else if (PlayerRef->GetMovementComponent()->CanVaultOrClimb(false))
                            return;
                    }
                    if (PlayerRef->GetCombatComponent()->HasCombatStance())
                    {
                        int currentCombatState = PlayerRef->GetCombatComponent()->GetCombatFSM().currentState()->Id;
                        bool canInterruptAction = (currentCombatState == ECombatState::Attack || currentCombatState == ECombatState::Parry) &&
                            !PlayerRef->GetCombatComponent()->GetCombatFSM().currentState()->Info.CheckAnimInfo(ECombatStateAnimInfo::MovementBlocked) &&
                            !PlayerRef->GetCombatComponent()->GetCombatFSM().currentState()->Info.CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed);
                        if (currentCombatState == ECombatState::Block || canInterruptAction)
                            PlayerRef->InterruptAnimation(ECCState::NoCC);
                    }
                    PlayerRef->Jump();
                    return;
                }
                else if (PlayerRef->GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (PlayerRef->GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingLedge:
                            break;
                        case ECustomMovementMode::ClimbingWall: {
                            if (InputComponent->GetAxisValue(FInputKeyNames::MoveForward)
                                > 0.f)
                                PlayerRef->GetMovementComponent()->ClimbWall();
                            else
                            {
                                const auto fwd = InputComponent->GetAxisValue(
                                  FInputKeyNames::MoveForward);
                                const auto rght =
                                  InputComponent->GetAxisValue(FInputKeyNames::MoveRight);
                                if (fwd != 0 || rght != 0)
                                {
                                    if (FMath::Abs(fwd) > FMath::Abs(rght))
                                        PlayerRef->GetMovementComponent()->JumpOffWall(
                                          EControlDirections::BackD);
                                    else if (rght > 0)
                                        PlayerRef->GetMovementComponent()->JumpOffWall(
                                          EControlDirections::RightD);
                                    else
                                        PlayerRef->GetMovementComponent()->JumpOffWall(
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
                else if (PlayerRef->GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Falling)
                {
                    if (PlayerRef->GetMovementComponent()->CheckCanHangAgain())
                        PlayerRef->GetMovementComponent()->HangOnFall();
                }
            }
            break;
        }
        default:
            return;
    }
}
void APlayerCharController::UnJumpAction() { PlayerRef->StopJumping(); }

void APlayerCharController::JumpRepAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!PlayerRef->GetССComponent()->HasCC()
                && !PlayerRef->GetMovementComponent()->CanMove())
            {
                if (PlayerRef->GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Falling)
                {
                    if (PlayerRef->GetMovementComponent()->CheckCanHangAgain())
                        PlayerRef->GetMovementComponent()->HangOnFall();
                }
            }
            break;
        }
    }
}

void APlayerCharController::DodgeAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!PlayerRef->GetССComponent()->HasCC()
                && PlayerRef->GetMovementComponent()->CanMove()
                && PlayerRef->GetCombatComponent()->WeaponDrawn())
            {
                if (PlayerRef->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Walking)
                    PlayerRef->GetMovementComponent()->DoDodge(GetInputDirection());
            }
            break;
        }
    }
}

void APlayerCharController::AttackOnAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
    case (EPawnGameplayState::InWorld): {
        if (!PlayerRef->GetССComponent()->HasCC())
            PlayerRef->GetCombatComponent()->OnAttack(true); // TODO check if ability is busy
    }
    }
}

void APlayerCharController::AttackOffAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!PlayerRef->GetССComponent()->HasCC())
                PlayerRef->GetCombatComponent()->OnAttack(false);
        }
    }
}

void APlayerCharController::BlockOnAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!PlayerRef->GetССComponent()->HasCC() && !PlayerRef->GetMovementComponent()->IsCrouching() &&
                PlayerRef->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Walking)
                PlayerRef->GetCombatComponent()->OnBlock(true);
        }
    }
}

void APlayerCharController::BlockOffAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!PlayerRef->GetССComponent()->HasCC() && !PlayerRef->GetMovementComponent()->IsCrouching())
                PlayerRef->GetCombatComponent()->OnBlock(false);
        }
    }
}

void APlayerCharController::WeaponAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!PlayerRef->GetССComponent()->HasCC() && PlayerRef->GetMovementComponent()->MovementMode != EMovementMode::MOVE_Custom)
                PlayerRef->GetCombatComponent()->OnDrawWeapon();
        }
    }
}

void APlayerCharController::AbilityOn()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            PlayerRef->GetCombatComponent()->OnUseAbility(true);
        }
    }
}

void APlayerCharController::AbilityOff()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            PlayerRef->GetCombatComponent()->OnUseAbility(false);
        }
    }
}

void APlayerCharController::CameraDistanceIn()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            PlayerRef->AdjustCameraZoom(PlayerRef->CameraZoomStep);
        }
    }
}

void APlayerCharController::CameraDistanceOut()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            PlayerRef->AdjustCameraZoom(-PlayerRef->CameraZoomStep);
        }
    }
}

void APlayerCharController::OpenCharacterMenu() 
{
    SetInputMode(UIMode);
}

void APlayerCharController::OpenInventory() 
{
    UE_LOG(LogTemp, Warning, TEXT("Open Inventory Menu"));

    if (CurrentGameplayState == EPawnGameplayState::InInventory)
    {
        return;
    }

    auto InventoryWidget = CreateWidget(PlayerRef->GetWorld(), InventoryWidgetClass, "InventoryWidget");
    CurrentWidget = Cast<UCharacterWidgetInterface>(InventoryWidget);

    if (InventoryWidget)
    {
        InventoryWidget->AddToViewport();
        //InventoryWidget->SetKeyboardFocus();        

        CurrentGameplayState = EPawnGameplayState::InInventory;
        SetInputMode(UIMode);
        SetShowMouseCursor(true);
    }
}

void APlayerCharController::OpenAbilityManager()
{
    UE_LOG(LogTemp, Warning, TEXT("Open Ability Menu"));

    if (CurrentGameplayState == EPawnGameplayState::InInventory)
    {
        return;
    }

    auto AbilityManagerWidget = CreateWidget(PlayerRef->GetWorld(), AbilityWidgetClass, "AbilityWidget");
    CurrentWidget = Cast<UCharacterWidgetInterface>(AbilityManagerWidget);

    if (AbilityManagerWidget)
    {
        AbilityManagerWidget->AddToViewport();
        //AbilityManagerWidget->SetKeyboardFocus();
        CurrentGameplayState = EPawnGameplayState::InInventory;
        SetInputMode(UIMode);
        SetShowMouseCursor(true);        
    }
}

void APlayerCharController::OpenMainMenu()
{
    UE_LOG(LogTemp, Warning, TEXT("Open Main Menu"));

    if ((CurrentGameplayState == EPawnGameplayState::InInventory)
    ||  (CurrentGameplayState == EPawnGameplayState::InMenu))
    {        
        if (CurrentWidget)
        {
            CurrentWidget->RemoveFromParent();
            CurrentWidget = nullptr;
        }
            
        return;
    }

    auto MenuWidget = CreateWidget(PlayerRef->GetWorld(), MenuWidgetClass, FName("MainMenu"));
    CurrentWidget = Cast<UCharacterWidgetInterface>(MenuWidget);
    //auto MenuWidget = PlayerWidgetTree->ConstructWidget<UCharacterWidgetInterface>(MenuWidgetClass, FName("MainMenu"));

    if (MenuWidget)
    {
        MenuWidget->AddToViewport();
        //MenuWidget->SetKeyboardFocus();
        CurrentGameplayState = EPawnGameplayState::InMenu;
        SetInputMode(UIMode);
        SetShowMouseCursor(true);
    }
}

void APlayerCharController::ReturnToGameplay() 
{    
    UE_LOG(LogTemp, Warning, TEXT("Close Menu"));
    if (CurrentGameplayState == EPawnGameplayState::InWorld)
        return;

    CurrentGameplayState = EPawnGameplayState::InWorld;    
    GEngine->ForceGarbageCollection();

    SetInputMode(GameMode); 
    SetShowMouseCursor(false);
}

void APlayerCharController::InteractAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            //Player->OnInteract(EGameplayReturnState::InWorld);
        }
    }
}

void APlayerCharController::OnPossess(APawn *pawn)
{
    APlayerController::OnPossess(pawn);
    PlayerRef = Cast<ACharacterPlayer>(pawn);

    CharacterHUD = Cast<UHUDCharacter>(CreateWidget(PlayerRef->GetWorld(), CharacterHUDClass, "HUD"));

    if (CharacterHUD)
        CharacterHUD->AddToViewport();

    ReturnToGameplay();
}

void APlayerCharController::OnUnPossess() 
{
    CharacterHUD->RemoveFromParent();
}
