#include "PlayerCharController.h"
// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharController.h"


#include "GameFramework/PlayerInput.h"
#include "Kismet/KismetMathLibrary.h"
#include "SaveSystem/CharacterSave.h"
#include "Character/CharacterPlayer.h"

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
    Player = Cast<ACharacterPlayer>(GetPawn());
    check(Player != nullptr && "APlayerCharController: No pawn");
    CurrentGameplayState = EPawnGameplayState::InWorld;    // debug
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
        InputComponent->BindAction(FInputKeyNames::Sprint,
                                   IE_Released,
                                   this,
                                   &APlayerCharController::UnSprintAction).bConsumeInput = false;
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
        //Note: Save TEST SHIT
        InputComponent->BindAction("Save", IE_Pressed, this, &APlayerCharController::SaveAction).bConsumeInput = false;
        InputComponent->BindAction("Load", IE_Pressed, this, &APlayerCharController::LoadAction).bConsumeInput = false;
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
    Player->CombatComponent->SpecialDirections &= 0xf & ~directions;
    if (directions & (ESpecialDirection::SLEFT | ESpecialDirection::SRIGHT))
        SideGotOld = true;
    else
        FaceGotOld = true;
}

void APlayerCharController::MoveForward(float AxisValue)
{
    check(Player != nullptr && "APlayerCharController: No pawn");
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (FMath::Abs(AxisValue) >= MovementDeadZone && !Player->CCStateManager.HasCC()
                && Player->GetMovementComponent()->CanMove())
            {
                if (Player->CombatComponent->HasCombatStance() && Player->CombatComponent->GetCombatFSM().isInState(ECombatState::Ability))// check if can move during cast, interrupt then
                {
                    if (Player->CombatComponent->CurrentAbility->MoveAbilities == 0)
                        Player->InterruptAnimation(ECCState::NoCC);
                }
                if (Player->GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Swimming)
                {
                    const FVector Direction =
                      UKismetMathLibrary::GetForwardVector(FRotator(
                        GetControlRotation().Pitch, GetControlRotation().Yaw, 0.f));
                    Player->AddMovementInput(Direction, AxisValue, false);
                }
                else if (Player->GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (Player->GetMovementComponent()->CustomMovementMode)
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
                    Player->AddMovementInput(Direction, AxisValue, false);
                    // special attack direction update
                    if (!FaceGotOld)
                    {
                        uint8 direction;
                        if (AxisValue > 0)
                            direction = ESpecialDirection::SFORWARD;
                        else
                            direction = ESpecialDirection::SBACK;
                        if (!(Player->CombatComponent->SpecialDirections & direction))
                        {
                            Player->CombatComponent->SpecialDirections &= 0xf & ~(ESpecialDirection::SFORWARD | ESpecialDirection::SBACK);
                            Player->CombatComponent->SpecialDirections |= direction;
                            GetWorld()->GetTimerManager().SetTimer(FaceTimer,
                                [&, direction]() { OnDirectionTimeout(direction); }, NewDirectionTime, false);
                        }
                    }
                }
            }
            else // reset special attack directions
            {
                Player->CombatComponent->SpecialDirections &= 0xf & ~(ESpecialDirection::SFORWARD | ESpecialDirection::SBACK);
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
    check(Player != nullptr && "APlayerCharController: No pawn");
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (FMath::Abs(AxisValue) >= MovementDeadZone && !Player->CCStateManager.HasCC()
                && Player->GetMovementComponent()->CanMove())
            {
                if (Player->CombatComponent->HasCombatStance() && Player->CombatComponent->GetCombatFSM().isInState(ECombatState::Ability))// check if can move during cast, interrupt then
                {
                    if (Player->CombatComponent->CurrentAbility->MoveAbilities == 0)
                        Player->InterruptAnimation(ECCState::NoCC);
                }
                if (Player->GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (Player->GetMovementComponent()->CustomMovementMode)
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
                      FRotator(0.f, Player->GetControlRotation().Yaw, 0.f);
                    const FVector Direction =
                      UKismetMathLibrary::GetRightVector(Rotation);
                    Player->AddMovementInput(Direction, AxisValue, false);
                    // special attack direction update
                    if (!SideGotOld)
                    {
                        uint8 direction;
                        if (AxisValue > 0)
                            direction = ESpecialDirection::SRIGHT;
                        else
                            direction = ESpecialDirection::SLEFT;
                        if (!(Player->CombatComponent->SpecialDirections & direction))
                        {
                            Player->CombatComponent->SpecialDirections &= 0xf & ~(ESpecialDirection::SRIGHT | ESpecialDirection::SLEFT);
                            Player->CombatComponent->SpecialDirections |= direction;
                            GetWorld()->GetTimerManager().SetTimer(SideTimer,
                                [&, direction]() { OnDirectionTimeout(direction); }, NewDirectionTime, false);
                        }
                    }
                    //if (GetWorld()->GetTimerManager().IsTimerActive(SideTimer))
                    //GetWorld()->GetTimerManager().ClearTimer(SideTimer);
                }
                if (Player->GetMovementComponent()->bWantToSprint && 
                    Player->CombatComponent->WeaponDrawn() && 
                    InputComponent->GetAxisValue(FInputKeyNames::MoveForward) <= 0.f)
                    Player->GetMovementComponent()->bWantToSprint = false;
            }
            else // reset special attack directions
            {
                Player->CombatComponent->SpecialDirections &= 0xf & ~(ESpecialDirection::SLEFT | ESpecialDirection::SRIGHT);
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
        }
        default:
            return;
    }
}

void APlayerCharController::RunAction()
{
    check(Player != nullptr && "APlayerCharController: No pawn");

    switch (CurrentGameplayState)
    {
        case (EPawnGameplayState::InWorld):    //todo учесть другие состояния
        {
            if (Player->GetMovementComponent()->bIsRunning)
                Player->GetMovementComponent()->bIsRunning = false;
            else
                Player->GetMovementComponent()->bIsRunning = true;
        }
        default:
            return;
    }
}

void APlayerCharController::SprintAction()    // TODO add equipment weight check
{
    check(Player != nullptr && "APlayerCharController: No pawn");

    switch (CurrentGameplayState)
    {
        case (EPawnGameplayState::InWorld):    //todo учесть другие состояния
        {
            if (Player->GetMovementComponent()->IsMovingOnGround()
                && !Player->CCStateManager.HasCC())
            {
                if ((Player->CombatComponent->WeaponDrawn()
                    && InputComponent->GetAxisValue(FInputKeyNames::MoveForward) <= 0.f) ||
                    Player->CombatComponent->IsBusy())
                    return;
                Player->GetMovementComponent()->bWantToSprint = true;
                if (Player->bIsCrouched)
                    Player->UnCrouch();
            }
        }
        default:
            return;
    }
}

void APlayerCharController::UnSprintAction()
{
    check(Player != nullptr && "APlayerCharController: No pawn");

    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (Player->GetMovementComponent()->bWantToSprint)
            {
                Player->GetMovementComponent()->bWantToSprint = false;
            }
        }
        default:
            return;
    }
}

void APlayerCharController::CrouchAction()
{
    check(Player != nullptr && "APlayerCharController: No pawn");
    Player->GetMovementComponent()->SetMovementMode(
      EMovementMode::MOVE_Walking);    // todo temp
    switch (CurrentGameplayState)
    {
        case (EPawnGameplayState::InWorld):    //todo учесть другие состояния
        {
            if (!Player->CCStateManager.HasCC()
                && !Player->GetMovementComponent()->IsRootOrTeleport())
            {
                if (Player->GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Walking)
                {
                    if (Player->CombatComponent->IsBusy())
                        return;
                    if (Player->GetMovementComponent()->IsCrouching())
                        Player->UnCrouch();
                    else
                        Player->Crouch();
                }
                else if (Player->GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (Player->GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingWall: {
                            Player->GetMovementComponent()->GetOffWall();
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
    check(Player != nullptr && "APlayerCharController: No pawn");

    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!Player->CCStateManager.HasCC()
                && Player->GetMovementComponent()->CanMove())
            {
                if (Player->GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Walking)
                {
                    if (InputComponent->GetAxisValue(FInputKeyNames::MoveForward) != 0.f
                        || InputComponent->GetAxisValue(FInputKeyNames::MoveRight) != 0.f)
                    {
                        if (Player->GetMovementComponent()->CanVaultOrClimb(true))
                            return;
                        else if (Player->GetMovementComponent()->CanVaultOrClimb(false))
                            return;
                    }
                    if (Player->CombatComponent->HasCombatStance())
                    {
                        int currentCombatState = Player->CombatComponent->GetCombatFSM().currentState()->Id;
                        bool canInterruptAction = (currentCombatState == ECombatState::Attack || currentCombatState == ECombatState::Parry) &&
                            !Player->CombatComponent->GetCombatFSM().currentState()->Info.CheckAnimInfo(ECombatStateAnimInfo::MovementBlocked) &&
                            !Player->CombatComponent->GetCombatFSM().currentState()->Info.CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed);
                        if (currentCombatState == ECombatState::Block || canInterruptAction)
                            Player->InterruptAnimation(ECCState::NoCC);
                    }
                    Player->Jump();
                    return;
                }
                else if (Player->GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Custom)
                {
                    //todo for parkur
                    switch (Player->GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingLedge:
                            break;
                        case ECustomMovementMode::ClimbingWall: {
                            if (InputComponent->GetAxisValue(FInputKeyNames::MoveForward)
                                > 0.f)
                                Player->GetMovementComponent()->ClimbWall();
                            else
                            {
                                const auto fwd = InputComponent->GetAxisValue(
                                  FInputKeyNames::MoveForward);
                                const auto rght =
                                  InputComponent->GetAxisValue(FInputKeyNames::MoveRight);
                                if (fwd != 0 || rght != 0)
                                {
                                    if (FMath::Abs(fwd) > FMath::Abs(rght))
                                        Player->GetMovementComponent()->JumpOffWall(
                                          EControlDirections::BackD);
                                    else if (rght > 0)
                                        Player->GetMovementComponent()->JumpOffWall(
                                          EControlDirections::RightD);
                                    else
                                        Player->GetMovementComponent()->JumpOffWall(
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
                else if (Player->GetMovementComponent()->MovementMode
                         == EMovementMode::MOVE_Falling)
                {
                    if (Player->GetMovementComponent()->CheckCanHangAgain())
                        Player->GetMovementComponent()->HangOnFall();
                }
            }
            break;
        }
        default:
            return;
    }
}
void APlayerCharController::UnJumpAction() { Player->StopJumping(); }

void APlayerCharController::JumpRepAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!Player->CCStateManager.HasCC()
                && !Player->GetMovementComponent()->CanMove())
            {
                if (Player->GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Falling)
                {
                    if (Player->GetMovementComponent()->CheckCanHangAgain())
                        Player->GetMovementComponent()->HangOnFall();
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
            if (!Player->CCStateManager.HasCC()
                && Player->GetMovementComponent()->CanMove() && Player->CombatComponent->WeaponDrawn())
            {
                if (Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Walking)
                    Player->GetMovementComponent()->DoDodge(GetInputDirection());
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
        if (!Player->CCStateManager.HasCC())
            Player->CombatComponent->OnAttack(true); // TODO check if ability is busy
    }
    }
}

void APlayerCharController::AttackOffAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!Player->CCStateManager.HasCC())
                Player->CombatComponent->OnAttack(false);
        }
    }
}

void APlayerCharController::BlockOnAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!Player->CCStateManager.HasCC() && !Player->GetMovementComponent()->IsCrouching() &&
                Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Walking)
                Player->CombatComponent->OnBlock(true);
        }
    }
}

void APlayerCharController::BlockOffAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!Player->CCStateManager.HasCC() && !Player->GetMovementComponent()->IsCrouching())
                Player->CombatComponent->OnBlock(false);
        }
    }
}

void APlayerCharController::WeaponAction()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (!Player->CCStateManager.HasCC() && Player->GetMovementComponent()->MovementMode != EMovementMode::MOVE_Custom)
                    Player->CombatComponent->OnDrawWeapon();
        }
    }
}

void APlayerCharController::AbilityOn()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            Player->CombatComponent->OnUseAbility(true);
        }
    }
}

void APlayerCharController::AbilityOff()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            Player->CombatComponent->OnUseAbility(false);
        }
    }
}

void APlayerCharController::CameraDistanceIn()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            Player->AdjustCameraZoom(Player->CameraZoomStep);
        }
    }
}

void APlayerCharController::CameraDistanceOut()
{
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            Player->AdjustCameraZoom(-Player->CameraZoomStep);
        }
    }
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


void APlayerCharController::SaveAction() 
{ 
    UCharacterSave *save = Cast<UCharacterSave>(UGameplayStatics::CreateSaveGameObject(UCharacterSave::StaticClass()));

    save->PlayerLocation = Player->GetActorLocation();

    UGameplayStatics::SaveGameToSlot(save, TEXT("MySlot"), 0);

    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Game Saved."));

}

void APlayerCharController::LoadAction() 
{
    UCharacterSave *save = Cast<UCharacterSave>(UGameplayStatics::CreateSaveGameObject(UCharacterSave::StaticClass()));

    save = Cast<UCharacterSave>(UGameplayStatics::LoadGameFromSlot("MySlot",0));
    
    Player->SetActorLocation(save->PlayerLocation);

    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Game Loaded."));
}
