// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Rules/GameRules.h"
#include "Attributes/DamageMap.h"
#include "AbilitySystem/EffectBase.h"
#include "AbilitySystem/AbstractAbility.h"
#include "CoreMinimal.h"
#include "ItemBase.h"
#include "UObject/NoExportTypes.h"

#include "ItemArmor.generated.h"

UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemArmor : public UItem
{
    GENERATED_BODY()

public:
    UItemArmor(const class FObjectInitializer &ObjectInititalizer);

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Weapon Stats")
    EArmorType ArmorType;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageAbsorb DamageLimit;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageResistance DamageResistance;
};
