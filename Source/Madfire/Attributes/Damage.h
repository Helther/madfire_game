// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/UserDefinedStruct.h"
#include "GameFramework/DamageType.h"
#include "Rules/GameRules.h"

#include "Damage.generated.h"

//TODO: Heal type and Mechanix

/**
 * 
 */

UCLASS(BlueprintType)
class UDamageBase : public UDamageType
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    EDamageType Type;
};


UCLASS(BlueprintType)
class UAbsoluteDamage : public UDamageBase
{
    GENERATED_BODY()
public:

};


UCLASS(BlueprintType, Blueprintable)
class UPhysicDamage : public UDamageBase
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    float Force;
};


UCLASS(BlueprintType, Blueprintable)
class UMagicDamage : public UDamageBase
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    EMagicSchool MagicSchool;
};
