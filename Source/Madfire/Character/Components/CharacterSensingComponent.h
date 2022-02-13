// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Perception/PawnSensingComponent.h"

#include "CharacterSensingComponent.generated.h"

/**
 * 
 */
UCLASS()
class MADFIRE_API UCharacterSensingComponent : public UPawnSensingComponent
{
    GENERATED_BODY()

    UCharacterSensingComponent(const class FObjectInitializer& ObjectInitilizer);

    UFUNCTION()
    void OnPawnSeen(APawn* SeenPawn);

    UFUNCTION()
    void OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume);
};
