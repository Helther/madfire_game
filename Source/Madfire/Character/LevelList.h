// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "../Attributes/Skills.h"
#include "../Attributes/Attributes.h"
#include "Components/ActorComponent.h"
#include "LevelList.generated.h"

constexpr auto DefaultHPBonus = 9;
constexpr auto MaxLevel = 30;
#define EXPERIENCE_FUCKING_EQUATION(x) ((x)*=2)

USTRUCT(BlueprintType)
struct FLevelInfo
{
	GENERATED_USTRUCT_BODY()
	FLevelInfo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
		float hpBonus;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
		FSkillList SkillList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
		FAttributes Attributes;
	//Talents
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ULevelComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	ULevelComponent(const class FObjectInitializer &ObjectInitilizer);

	UFUNCTION(BlueprintCallable, Category = "Character Attributes")
		void ResetLevel();

	UFUNCTION(BlueprintCallable, Category = "Character Attributes")
		void LevelUp(const FLevelInfo &levelInfo);
	UFUNCTION(BlueprintPure, Category = "Character Attributes")
		FORCEINLINE int32 GetLevelNum() const { return LevelList.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Character Attributes")
		void update();
	UFUNCTION(BlueprintPure, Category = "Character Attributes")
		FORCEINLINE bool CanLevelUp() const { return (CurrentExpirience >= RequiredExpirience) && (LevelList.Num() <= MaxLevel); }

	UFUNCTION(BlueprintPure, Category = "Character Attributes")
		static FORCEINLINE uint8 GetMaxPossibleLevel() { return MaxLevel; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
		float CurrentExpirience;
	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes")
		float LevelHealthBonus;
	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes")
		FAttributes LevelAttributes;
	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes")
		FSkillList LevelSkills;
	
	UPROPERTY(BlueprintReadOnly , Category = "Character Attributes")
		float RequiredExpirience = 100;
	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes")
		TArray<FLevelInfo> LevelList;
};
