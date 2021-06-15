// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Rules/GameRules.h"
#include "Attributes/Damage.h"
#include "Attributes/DamageMap.h"
#include "CoreMinimal.h"
#include "ItemBase.h"
#include "UObject/NoExportTypes.h"

#include "ItemWeapon.generated.h"

UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemWeapon : public UItem
{
    GENERATED_BODY()

public:
    UItemWeapon(const class FObjectInitializer &ObjectInititalizer);

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Weapon Stats")
    EWeaponType WeaponType;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Weapon Stats")
    EWeaponSize WeaponSize;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    float DamageValue;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Weapon Stats")
    TSubclassOf<UPhysicDamage> DamageType;

    //For Shields
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageAbsorb DamageLimit;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageResistance DamageResistance;
};
