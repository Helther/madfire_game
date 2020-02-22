// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePlayerController.h"
#include "CharacterBase.h"
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
	PawnOwner = Cast<ACharacterBase>(GetPawn());
	check(PawnOwner != nullptr && "No pawn");
    CurrentGameplayState = EPawnGameplayState::InWorld; //care
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
    for(int32 i = 0;i < KeyListTest.Num();++i)
    {
        if(IsInputKeyDown(KeyListTest[i].Key))
        {
            UE_LOG(LogTemp, Warning, TEXT("Pressing %s"), *ActionKey.ToString());
            return true;
        }
    }
    return false;
}

void ABasePlayerController::MoveForward(float AxisValue)
{
	check(PawnOwner != nullptr && "No pawn");
	switch(CurrentGameplayState)
	{
		case(EPawnGameplayState::InWorld):
		{
			if (AxisValue != 0.0f && PawnOwner->CurrentCCState == ECCState::NONE)
			{
				if (PawnOwner->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Swimming)
				{
					const FVector Direction = UKismetMathLibrary::GetForwardVector(FRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, 0.f));
					PawnOwner->AddMovementInput(Direction, AxisValue, false);
                    UE_LOG(LogTemp, Warning, TEXT("waaalk"));
				}
				else if (PawnOwner->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
				{
					//to do for parkur
					switch(PawnOwner->GetMovementComponent()->CustomMovementMode)
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
                    PawnOwner->AddMovementInput(Direction, AxisValue, false);
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
	check(PawnOwner != nullptr && "No pawn");
	switch (CurrentGameplayState)
	{
		case(EPawnGameplayState::InWorld):
		{
			if (AxisValue != 0.0f && PawnOwner->CurrentCCState == ECCState::NONE)
			{
				if (PawnOwner->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
				{
                    //to do for parkur
                    switch(PawnOwner->GetMovementComponent()->CustomMovementMode)
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
					const FRotator Rotation = FRotator(0.f, PawnOwner->GetControlRotation().Yaw, 0.f);
					const FVector Direction = UKismetMathLibrary::GetRightVector(Rotation);
					PawnOwner->AddMovementInput(Direction, AxisValue, false);
				}
            }
		}
	default:
		return;
	}
}

void ABasePlayerController::CameraPitch(float AxisValue)
{
	switch (CurrentGameplayState)
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
	switch (CurrentGameplayState)
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
	check(PawnOwner != nullptr && "No pawn");

	switch (CurrentGameplayState)
	{
	case(EPawnGameplayState::InWorld):
	{
		if (PawnOwner->GetMovementComponent()->bIsRunning)
			PawnOwner->GetMovementComponent()->bIsRunning = false;
		else
			PawnOwner->GetMovementComponent()->bIsRunning = true;
	}
	default:
		return;
	}
}

void ABasePlayerController::SprintAction()///add equipment weight check
{
	check(PawnOwner != nullptr && "No pawn");

	switch (CurrentGameplayState)
	{
	case(EPawnGameplayState::InWorld):
	{
		if (PawnOwner->GetMovementComponent()->IsMovingOnGround() && PawnOwner->CurrentCCState == ECCState::NONE)
		{
            if(PawnOwner->GetCombatComponent()->IsWeaponDrawn && InputComponent->GetAxisValue(FInputKeyNames::MoveForward) <= 0.0f)
                return;
			PawnOwner->GetMovementComponent()->bWantToSprint = true;
			if (PawnOwner->bIsCrouched)
				PawnOwner->UnCrouch();
		}
	}
	default:
		return;
	}
}

void ABasePlayerController::UnSprintAction()
{
	check(PawnOwner != nullptr && "No pawn");

	switch (CurrentGameplayState)
	{
	case(EPawnGameplayState::InWorld):
	{
		if (PawnOwner->GetMovementComponent()->bWantToSprint)
		{
			PawnOwner->GetMovementComponent()->bWantToSprint = false;
		}
	}
	default:
		return;
	}
}

void ABasePlayerController::CrouchAction()//to do
{
	check(PawnOwner != nullptr && "No pawn");

	switch (CurrentGameplayState)
	{
		case(EPawnGameplayState::InWorld):
		{
			if (PawnOwner->CurrentCCState == ECCState::NONE)
			{
				if (PawnOwner->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Walking)
				{
					if (PawnOwner->GetMovementComponent()->IsCrouching())
						PawnOwner->UnCrouch();
					else if (!PawnOwner->GetMovementComponent()->IsSwimming())
						PawnOwner->Crouch();
				}
				else if (PawnOwner->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
				{
                    //to do for parkur
                    switch (PawnOwner->GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingWall:
                            PawnOwner->GetMovementComponent()->GetOffWall();
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

void ABasePlayerController::JumpAction()//to do
{
	check(PawnOwner != nullptr && "No pawn");

	switch (CurrentGameplayState)
	{
		case(EPawnGameplayState::InWorld):
		{
			if (PawnOwner->CurrentCCState == ECCState::NONE)
			{
				if (PawnOwner->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Walking)
				{
                    if(InputComponent->GetAxisValue(FInputKeyNames::MoveForward) != 0.0f || InputComponent->GetAxisValue(FInputKeyNames::MoveRight) != 0.f)
                    {
                        if(PawnOwner->GetMovementComponent()->CanVaultOrClimb(true))
                            return;
                        else if(PawnOwner->GetMovementComponent()->CanVaultOrClimb(false))
                            return;
                    }
                    PawnOwner->Jump(); ///add stamina calculation and usage
				}
				else if (PawnOwner->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
				{
					//to do for parkur
                    switch (PawnOwner->GetMovementComponent()->CustomMovementMode)
                    {
                        case ECustomMovementMode::ClimbingLedge:
                            //PawnOwner->GetMovementComponent()->SetMovementMode(MOVE_Walking);///debug
                            break;
                        case ECustomMovementMode::ClimbingWall:
                            PawnOwner->GetMovementComponent()->ClimbWall();
                            break;
                        case ECustomMovementMode::ClimbingLadder:
                        case ECustomMovementMode::Dodge:
                        case ECustomMovementMode::Slide:
                        default:
                            break;
                    }

				}
                else if (PawnOwner->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Falling && PawnOwner->GetMovementComponent()->bCanHangAgain)
                {
                    PawnOwner->GetMovementComponent()->HangOnWall();
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
    switch (CurrentGameplayState)
    {
        case(EPawnGameplayState::InWorld):
        {
            if (PawnOwner->CurrentCCState == ECCState::NONE)
            {
                if (PawnOwner->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Falling && PawnOwner->GetMovementComponent()->bCanHangAgain)
                {
                    PawnOwner->GetMovementComponent()->HangOnWall();
                }
            }
            break;
        }
    }
}


