// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Rules/GameRules.h"
#include "Engine/UserDefinedStruct.h"

#include "DamageMap.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FDamageResistance
{
    GENERATED_BODY()

    FDamageResistance &operator+=(const FDamageResistance &resistance);
    FDamageResistance operator+(const FDamageResistance &resistance);

    UPROPERTY(EditAnywhere, Category = Impact)
    uint16 DamageMap[(int32)EDamageType::Max];
};

USTRUCT(BlueprintType)
struct FDamageAbsorb
{
    GENERATED_BODY()

    FDamageAbsorb &operator+=(const FDamageAbsorb &resistance);
    FDamageAbsorb operator+(const FDamageAbsorb &resistance);

    UPROPERTY(EditAnywhere, Category = Impact)
    int8 DamageMap[(int32)EDamageType::Max];
};


UCLASS()
class UDamageMapHelper : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Damage Map")
    static FORCEINLINE int32 GetResistanceValue(const FDamageResistance &dmgMap,
                                                EDamageType DamageName)
    {
        return dmgMap.DamageMap[(int32)DamageName];
    }

    UFUNCTION(BlueprintCallable, Category = "Damage Map")
    static void SetResistanceValue(FDamageResistance &dmgMap, EDamageType DamageName,
                                   int32 Value)
    {
        dmgMap.DamageMap[(int32)DamageName] = (uint8)Value;
    }

    UFUNCTION(BlueprintPure, Category = "Damage Map")
    static FORCEINLINE int32 GetAbsorbValue(const FDamageAbsorb &dmgMap,
                                            EDamageType DamageName)
    {
        return dmgMap.DamageMap[(int32)DamageName];
    }

    UFUNCTION(BlueprintCallable, Category = "Damage Map")
    static void SetAbsorbValue(FDamageAbsorb &dmgMap, EDamageType DamageName, int32 Value)
    {
        dmgMap.DamageMap[(int32)DamageName] = (uint8)Value;
    }
};
