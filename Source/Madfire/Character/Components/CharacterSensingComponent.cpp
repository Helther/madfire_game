// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/CharacterSensingComponent.h"


UCharacterSensingComponent::UCharacterSensingComponent(const FObjectInitializer& ObjectInitilizer)
    : Super(ObjectInitilizer)
{
    PrimaryComponentTick.bCanEverTick = true;
    OnSeePawn.AddDynamic(this, &UCharacterSensingComponent::OnPawnSeen);
    OnHearNoise.AddDynamic(this, &UCharacterSensingComponent::OnNoiseHeard);
}

void UCharacterSensingComponent::OnPawnSeen(APawn* SeenPawn)
{
    //TODO
}

void UCharacterSensingComponent::OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
    //TODO
}
