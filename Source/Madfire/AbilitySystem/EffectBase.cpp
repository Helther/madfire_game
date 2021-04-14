// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/EffectBase.h"
#include "Character/CharacterBase.h"




void UEffectBase::OnUpdate_Implementation(ACharacterBase *Target, float DeltaTime)
{
    CurrentDuration-=DeltaTime;
}
