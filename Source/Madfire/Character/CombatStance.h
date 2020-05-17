// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ComboFSM.h"
#include "CombatStance.generated.h"

/**
 * 
 */
UCLASS()
class MADFIRE_API UCombatStance : public UDataAsset
{
	GENERATED_BODY()
	
public:
	////////////////////////// combat mode speeds//////////////////////////

	UPROPERTY(EditAnywhere, Category = "Character Combat Speeds")
        float MaxCastingSpeed{}; //Max move speed during cast
	UPROPERTY(EditAnywhere, Category = "Character Combat Speeds")
		float MaxCombatRunSpeed{};

	UPROPERTY(EditAnywhere, Category = "Character Combat Speeds")
		float MaxCombatCrouchedSpeed{};

	UPROPERTY(EditAnywhere, Category = "Character Combat Speeds")
		float MaxCombatSprintSpeed{};
	
    UPROPERTY(EditAnywhere, Category = "Character Combat Speeds")
        float MaxBlockSpeed{}; //Max move speed during blocking


};
