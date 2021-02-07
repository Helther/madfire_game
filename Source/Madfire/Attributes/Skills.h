// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Rules/GameRules.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Skills.generated.h"

/**
 * 
 */


USTRUCT(BlueprintType)
struct FSkillList
{
    GENERATED_BODY()

    FSkillList();

    FSkillList &operator+=(const FSkillList &rhs);
    FSkillList operator+(const FSkillList &rhs);

    UPROPERTY(EditAnywhere, Category = "Skill")
    int8 SkillMap[(int32)ESkill::Max];

    static const EAttributes ModifierMap[(int32)ESkill::Max];
};

UCLASS()
class USkillHelper : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Skill Operations")
    static FORCEINLINE int32 GetValue(const FSkillList &skill, ESkill SkillName)
    {
        return skill.SkillMap[(int32)SkillName];
    }

    UFUNCTION(BlueprintPure, Category = "Skill Operations")
    static FORCEINLINE EAttributes GetModifier(ESkill SkillName)
    {
        return FSkillList::ModifierMap[(int32)SkillName];
    }

    UFUNCTION(BlueprintCallable, Category = "Skill Operations")
    static void SetValue(FSkillList &skill, ESkill SkillName, int32 value);
};