// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePlayerController.h"
#include "CharacterPlayer.h"///заменить на игрового перса
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerInput.h"

const FName FInputKeyNames::MoveForward = "MoveForward";
const FName FInputKeyNames::MoveRight = "MoveRight";
const FName FInputKeyNames::Run = "Run";
const FName FInputKeyNames::Sprint = "Sprint";
const FName FInputKeyNames::Crouch = "Crouch";
const FName FInputKeyNames::Jump = "Jump";
const FName FInputKeyNames::CameraPitch = "CameraPitch";
const FName FInputKeyNames::CameraYaw = "CameraYaw";
const FName FInputKeyNames::ClimbDownTest = "ClimbDownTest";

ABasePlayerController::ABasePlayerController()
{

}

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();
	Player = Cast<ACharacterPlayer>(GetPawn());
	check(Player != nullptr && "ABasePlayerController: No pawn");
    CurrentGameplayState = EPawnGameplayState::InWorld; // debug
}
void ABasePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//////////////////////////////////////////////////////////////////////////
// Input
void ABasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent)
	{
        InputComponent->BindAction(FInputKeyNames::Run, IE_Pressed, this, &ABasePlayerController::RunAction);
        InputComponent->BindAction(FInputKeyNames::Sprint, IE_Pressed, this, &ABasePlayerController::SprintAction);
        InputComponent->BindAction(FInputKeyNames::Sprint, IE_Released, this, &ABasePlayerController::UnSprintAction);
        InputComponent->BindAction(FInputKeyNames::Crouch, IE_Pressed, this, &ABasePlayerController::CrouchAction);
        InputComponent->BindAction(FInputKeyNames::Jump, IE_Pressed, this, &ABasePlayerController::JumpAction);
        InputComponent->BindAxis(FInputKeyNames::MoveForward, this, &ABasePlayerController::MoveForward);
        InputComponent->BindAxis(FInputKeyNames::MoveRight, this, &ABasePlayerController::MoveRight);
        InputComponent->BindAxis(FInputKeyNames::CameraPitch, this, &ABasePlayerController::CameraPitch);
        InputComponent->BindAxis(FInputKeyNames::CameraYaw, this, &ABasePlayerController::CameraYaw);
        InputComponent->BindAction(FInputKeyNames::Jump, IE_Repeat, this ,&ABasePlayerController::JumpRepAction);
	}
}

bool ABasePlayerController::IsActionKeyPressed(const FName &ActionKey)
{
    TArray<FInputActionKeyMapping> KeyListTest = PlayerInput->GetKeysForAction(ActionKey);
    if(KeyListTest.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No keys set for action %s"), *ActionKey.ToString());
        return false;
    }
    for(const auto i : KeyListTest)
    {
        if(IsInputKeyDown(i.Key))
        {
            UE_LOG(LogTemp, Warning, TEXT("Pressing %s"), *ActionKey.ToString());
            return true;
        }
    }
    return false;
}

void ABasePlayerController::MoveForward(float AxisValue)
{
	check(Player != nullptr && "ABasePlayerController: No pawn");
	switch(CurrentGameplayState)//todo учесть другие состояния
	{
		case(EPawnGameplayState::InWorld):
		{
			if (AxisValue != 0.0f && Player->CurrentCCState == ECCState::NONE)
			{
				if (Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Swimming)
				{
					const FVector Direction = UKismetMathLibrary::GetForwardVector(FRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, 0.f));
					Player->AddMovementInput(Direction, AxisValue, false);
				}
				else if (Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
				{
					//todo for parkur
					switch(Player->GetMovementComponent()->CustomMovementMode)
					{
						case(ECustomMovementMode::ClimbingLedge):
						case(ECustomMovementMode::Slide):
						case(ECustomMovementMode::Dodge):
							return;
						default:
							return;
					}
				}
				else
				{
					FRotator Rotation = FRotator(0.f, GetControlRotation().Yaw, 0.f);
					const FVector Direction = UKismetMathLibrary::GetForwardVector(Rotation);
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

void ABasePlayerController::MoveRight(float AxisValue)
{
	check(Player != nullptr && "ABasePlayerController: No pawn");
	switch (CurrentGameplayState)//todo учесть другие состояния
	{
		case(EPawnGameplayState::InWorld):
		{
			if (AxisValue != 0.0f && Player->CurrentCCState == ECCState::NONE)
			{
				if (Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
				{
                    //todo for parkur
                    switch(Player->GetMovementComponent()->CustomMovementMode)
                    {
                        case(ECustomMovementMode::ClimbingLedge):
                        case(ECustomMovementMode::Slide):
                        case(ECustomMovementMode::Dodge):
                            return;
                        default:
                            return;
                    }
				}
				else
				{
					const FRotator Rotation = FRotator(0.f, Player->GetControlRotation().Yaw, 0.f);
					const FVector Direction = UKismetMathLibrary::GetRightVector(Rotation);
					Player->AddMovementInput(Direction, AxisValue, false);
				}
            }
		}
	default:
		return;
	}
}

void ABasePlayerController::CameraPitch(float AxisValue)
{
	switch (CurrentGameplayState)//todo учесть другие состояния
	{
	case(EPawnGameplayState::InWorld):
	case(EPawnGameplayState::InMountMode):
	{
        AddPitchInput(AxisValue * GetWorld()->GetDeltaSeconds());
	}
	default:
		return;
	}
}

void ABasePlayerController::CameraYaw(float AxisValue)
{
	switch (CurrentGameplayState)//todo учесть другие состояния
	{
		case(EPawnGameplayState::InWorld):
		case(EPawnGameplayState::InMountMode):
		{
            AddYawInput(AxisValue * GetWorld()->GetDeltaSeconds());
		}
		default:
			return;
	}
}

void ABasePlayerController::RunAction()
{
	check(Player != nullptr && "ABasePlayerController: No pawn");

	switch (CurrentGameplayState)
	{
	case(EPawnGameplayState::InWorld)://todo учесть другие состояния
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

void ABasePlayerController::SprintAction()///add equipment weight check
{
	check(Player != nullptr && "ABasePlayerController: No pawn");

	switch (CurrentGameplayState)
	{
	case(EPawnGameplayState::InWorld)://todo учесть другие состояния
	{
		if (Player->GetMovementComponent()->IsMovingOnGround() && Player->CurrentCCState == ECCState::NONE)
		{
            if(Player->GetCombatComponent()->IsWeaponDrawn && InputComponent->GetAxisValue(FInputKeyNames::MoveForward) <= 0.0f)
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

void ABasePlayerController::UnSprintAction()
{
	check(Player != nullptr && "ABasePlayerController: No pawn");

	switch (CurrentGameplayState)//todo учесть другие состояния
	{
	case(EPawnGameplayState::InWorld):
	{
		if (Player->GetMovementComponent()->bWantToSprint)
		{
			Player->GetMovementComponent()->bWantToSprint = false;
		}
	}
	default:
		return;
	}
}

void ABasePlayerController::CrouchAction()
{
	check(Player != nullptr && "ABasePlayerController: No pawn");

	switch (CurrentGameplayState)
	{
		case(EPawnGameplayState::InWorld)://todo учесть другие состояния
		{
			if (Player->CurrentCCState == ECCState::NONE)
			{
				if (Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Walking)
				{
					if (Player->GetMovementComponent()->IsCrouching())
						Player->UnCrouch();
					else 
						Player->Crouch();///todo вызывать из компонента движения
				}
				else if (Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
				{
                    //todo for parkur
                    switch (Player->GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingWall:
                            Player->GetMovementComponent()->GetOffWall();
                            break;
                        case ECustomMovementMode::ClimbingLedge:
                        case ECustomMovementMode::ClimbingLadder:
                        case ECustomMovementMode::Dodge:
                        case ECustomMovementMode::Slide:
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

void ABasePlayerController::JumpAction()//todo
{
	check(Player != nullptr && "ABasePlayerController: No pawn");

	switch (CurrentGameplayState)//todo учесть другие состояния
	{
		case(EPawnGameplayState::InWorld):
		{
			if (Player->CurrentCCState == ECCState::NONE)
			{
				if (Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Walking)
				{
                    if(InputComponent->GetAxisValue(FInputKeyNames::MoveForward) != 0.0f || InputComponent->GetAxisValue(FInputKeyNames::MoveRight) != 0.f)
                    {
                        if(Player->GetMovementComponent()->CanVaultOrClimb(true))
                            return;
                        else if(Player->GetMovementComponent()->CanVaultOrClimb(false))
                            return;
                    }
                    Player->Jump();
					return;
				}
				else if (Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
				{
					//todo for parkur
                    switch (Player->GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingLedge:
                            //Player->GetMovementComponent()->SetMovementMode(MOVE_Walking);///debug
                            break;
                        case ECustomMovementMode::ClimbingWall:
                            Player->GetMovementComponent()->ClimbWall();
                            break;
                        case ECustomMovementMode::ClimbingLadder:
                        case ECustomMovementMode::Dodge:
                        case ECustomMovementMode::Slide:
                        default:
                            break;
                    }

				}
                else if (Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Falling)
                {
					if(!Player->GetMovementComponent()->CheckCanHangAgain())
                    	Player->GetMovementComponent()->HangOnWall();
                }
			}
            break;
		}
		default:
			return;
    }
}

void ABasePlayerController::JumpRepAction()
{
    switch (CurrentGameplayState)//todo учесть другие состояния
    {
        case(EPawnGameplayState::InWorld):
        {
            if (Player->CurrentCCState == ECCState::NONE)
            {
                if (Player->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Falling)
                {
					if(!Player->GetMovementComponent()->CheckCanHangAgain())
                    	Player->GetMovementComponent()->HangOnWall();
                }
            }
            break;
        }
    }
}


