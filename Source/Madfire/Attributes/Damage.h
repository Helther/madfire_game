// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/UserDefinedStruct.h"
#include "Rules/GameRules.h"
#include "GameFramework/DamageType.h"
#include "Damage.generated.h"

/**
 * 
 */

UCLASS(BlueprintType, Blueprintable)
class UPhysicDamage : public UDamageType
{
	GENERATED_BODY()
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float       Force;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EDamageType	Type;
};


UCLASS(BlueprintType, Blueprintable)
class UMagicDamage : public UDamageType
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EDamageType Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EMagicSchool MagicSchool;
};


