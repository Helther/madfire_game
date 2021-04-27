// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharController.h"


#include "GameFramework/PlayerInput.h"
#include "Kismet/KismetMathLibrary.h"

const FName FInputKeyNames::MoveForward   = "MoveForward";
const FName FInputKeyNames::MoveRight     = "MoveRight";
const FName FInputKeyNames::Run           = "Run";
const FName FInputKeyNames::Sprint        = "Sprint";
const FName FInputKeyNames::Crouch        = "Crouch";
const FName FInputKeyNames::Jump          = "Jump";
const FName FInputKeyNames::Dodge         = "Dodge";
const FName FInputKeyNames::CameraPitch   = "CameraPitch";
const FName FInputKeyNames::CameraYaw     = "CameraYaw";
const FName FInputKeyNames::ClimbDownTest = "ClimbDownTest";

APlayerCharController::APlayerCharController() {}

EControlDirections APlayerCharController::GetInputDirection()
{
    const auto fwd = InputComponent->GetAxisValue(FInputKeyNames::MoveForward);
    const auto rght = InputComponent->GetAxisValue(FInputKeyNames::MoveRight);
    if (fwd > MovementDeadZone)
    {
        if (rght > MovementDeadZone)
            return EControlDirections::FRightD;
        else if  (rght < MovementDeadZone)
            return EControlDirections::FLeftD;
        else
            return EControlDirections::ForwardD;
    }
    else if (fwd < -MovementDeadZone)
    {
        if (rght > MovementDeadZone)
            return EControlDirections::BRightD;
        else if  (rght < MovementDeadZone)
            return EControlDirections::BLeftD;
        else
            return EControlDirections::BackD;
    }
    else if (rght > MovementDeadZone)
    {
        if (fwd > MovementDeadZone)
            return EControlDirections::FRightD;
        else if  (fwd < MovementDeadZone)
            return EControlDirections::BRightD;
        else
            return EControlDirections::RightD;
    }
    else if (rght < -MovementDeadZone)
    {
        if (fwd > MovementDeadZone)
            return EControlDirections::FLeftD;
        else if  (fwd < MovementDeadZone)
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
          FInputKeyNames::Run, IE_Pressed, this, &APlayerCharController::RunAction);
        InputComponent->BindAction(
          FInputKeyNames::Sprint, IE_Pressed, this, &APlayerCharController::SprintAction);
        InputComponent->BindAction(FInputKeyNames::Sprint,
                                   IE_Released,
                                   this,
                                   &APlayerCharController::UnSprintAction);
        InputComponent->BindAction(
          FInputKeyNames::Crouch, IE_Pressed, this, &APlayerCharController::CrouchAction);
        InputComponent->BindAction(
          FInputKeyNames::Jump, IE_Pressed, this, &APlayerCharController::JumpAction);
        InputComponent->BindAction(
          FInputKeyNames::Jump, IE_Released, this, &APlayerCharController::UnJumpAction);
        InputComponent->BindAction(
          FInputKeyNames::Dodge, IE_Pressed, this, &APlayerCharController::DodgeAction);
        InputComponent->BindAxis(
          FInputKeyNames::MoveForward, this, &APlayerCharController::MoveForward);
        InputComponent->BindAxis(
          FInputKeyNames::MoveRight, this, &APlayerCharController::MoveRight);
        InputComponent->BindAxis(
          FInputKeyNames::CameraPitch, this, &APlayerCharController::CameraPitch);
        InputComponent->BindAxis(
          FInputKeyNames::CameraYaw, this, &APlayerCharController::CameraYaw);
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

void APlayerCharController::MoveForward(float AxisValue)
{
    check(Player != nullptr && "APlayerCharController: No pawn");
    switch (CurrentGameplayState)    //todo учесть другие состояния
    {
        case (EPawnGameplayState::InWorld): {
            if (AxisValue != 0.0f && Player->CurrentCCState == ECCState::NoCC
                && !Player->GetMovementComponent()->IsRootOrTeleport())
            {
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
                    FRotator Rotation = FRotator(0.f, GetControlRotation().Yaw, 0.f);
                    const FVector Direction =
                      UKismetMathLibrary::GetForwardVector(Rotation);
                    //UE_LOG(LogTemp, Warning, TEXT("Forward vector %s"), *Direction.ToString());
                    Player->AddMovementInput(Direction, AxisValue, false);
                }
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
            if (AxisValue != 0.0f && Player->CurrentCCState == ECCState::NoCC
                && !Player->GetMovementComponent()->IsRootOrTeleport())
            {
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
                }
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

void APlayerCharController::SprintAction()    ///add equipment weight check
{
    check(Player != nullptr && "APlayerCharController: No pawn");

    switch (CurrentGameplayState)
    {
        case (EPawnGameplayState::InWorld):    //todo учесть другие состояния
        {
            if (Player->GetMovementComponent()->IsMovingOnGround()
                && Player->CurrentCCState == ECCState::NoCC)
            {
                if (Player->CombatComponent->IsWeaponDrawn
                    && InputComponent->GetAxisValue(FInputKeyNames::MoveForward) <= 0.f)
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
            if (Player->CurrentCCState == ECCState::NoCC
                && !Player->GetMovementComponent()->IsRootOrTeleport())
            {
                if (Player->GetMovementComponent()->MovementMode
                    == EMovementMode::MOVE_Walking)
                {
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
            if (Player->CurrentCCState == ECCState::NoCC
                && !Player->GetMovementComponent()->IsRootOrTeleport())
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
            if (Player->CurrentCCState == ECCState::NoCC
                && !Player->GetMovementComponent()->IsRootOrTeleport())
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
            if (Player->CurrentCCState == ECCState::NoCC
                && !Player->GetMovementComponent()->IsRootOrTeleport()) // todo учесть комбат
            {
                if (Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Walking)
                    Player->GetMovementComponent()->DoDodge(GetInputDirection());
            }
            break;
        }
    }
}
