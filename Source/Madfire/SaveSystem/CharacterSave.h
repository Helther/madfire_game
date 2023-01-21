// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CharacterSave.generated.h"

/**
 * 
 */
UCLASS()
class MADFIRE_API UCharacterSave : public USaveGame
{
	GENERATED_BODY()
	
public:
    UCharacterSave();

	UPROPERTY(EditAnywhere)
    FVector PlayerLocation;

};
