// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstance.generated.h"


UENUM(BlueprintType)
// Animation classification types with values as priorities
enum class AnimTypes : uint8
{
	Block = 0				UMETA(DisplayName = "Block"),
	Parkour = 1			UMETA(DisplayName = "Parkour"),
	SpellCast = 2		UMETA(DisplayName = "SpellCast"),
	Attack	= 3			UMETA(DisplayName = "Attack"),
	Dodge	= 4				UMETA(DisplayName = "Dodge"),
	CC = 5					UMETA(DisplayName = "Character Control")
};

UCLASS()
class MADFIRE_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Character")	
		class ACharacterBase* Character;

private:
	void NativeUpdateAnimation(float DeltaSeconds) override;



};
