#pragma once

#include "Attributes/Attributes.h"
#include "Attributes/Skills.h"
#include "Object.h"
#include "CharacterLevelInfo.generated.h"

constexpr auto DefaultHPBonus = 9;

USTRUCT(BlueprintType)
struct FCharacterLevelInfo
{
    GENERATED_USTRUCT_BODY()

    FCharacterLevelInfo();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    float HealthBonus;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    FSkillList SkillList;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    FAttributeSet Attributes;
    //todo add feats
};
