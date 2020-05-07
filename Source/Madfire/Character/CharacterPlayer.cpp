// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterPlayer.h"
#include "BasePlayerController.h"

ACharacterPlayer::ACharacterPlayer(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{CurrentCCState = ECCState::NONE;}

// Called to bind functionality to input
void ACharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACharacterPlayer::OnWalkingOffLedge_Implementation(const FVector &PreviousFloorImpactNormal, 
													const FVector &PreviousFloorContactNormal, 
													const FVector &PreviousLocation, 
													float TimeDelta)
{
    UE_LOG(LogTemp, Warning, TEXT("OnWalkingOffLedge"));
    ABasePlayerController *PlayerController = Cast<ABasePlayerController>(GetController());
    if(PlayerController->IsActionKeyPressed(FInputKeyNames::ClimbDownTest))
    {
        UE_LOG(LogTemp, Warning, TEXT("climb down"));
        GetMovementComponent()->ClimbDownWall();
    }
}
