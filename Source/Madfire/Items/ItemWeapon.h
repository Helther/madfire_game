// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemBase.h"
#include "Attributes/Damage.h"
#include "../Rules/GameRules.h"
#include "ItemWeapon.generated.h"

UCLASS(config = Game, EditInlineNew, BlueprintType, Blueprintable)
class UItemWeapon : public UItemBase
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
};

