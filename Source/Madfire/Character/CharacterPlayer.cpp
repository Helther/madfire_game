// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterPlayer.h"

#include "PlayerCharController.h"

ACharacterPlayer::ACharacterPlayer(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{

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

