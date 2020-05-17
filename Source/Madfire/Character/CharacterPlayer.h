// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "CharacterPlayer.generated.h"

/**
 * 
 */
UCLASS()
class MADFIRE_API ACharacterPlayer : public ACharacterBase
{
	GENERATED_BODY()

public:
	ACharacterPlayer(const FObjectInitializer& ObjectInitializer);
    
protected:
	// Called to bind functionality to input 
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual void OnWalkingOffLedge_Implementation( const FVector & PreviousFloorImpactNormal,
                                                   const FVector & PreviousFloorContactNormal,
                                                   const FVector & PreviousLocation,
                                                   float TimeDelta) override;
};
