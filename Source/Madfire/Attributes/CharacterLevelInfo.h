#pragma once

#include "Attributes/Attributes.h"
#include "Attributes/Skills.h"
#include "Attributes/DamageMap.h"
#include "Object.h"
#include "CharacterLevelInfo.generated.h"

constexpr auto DefaultHPBonus = 9;

USTRUCT(BlueprintType)
struct FCharacterLevelInfo
{
    GENERATED_USTRUCT_BODY()

    FCharacterLevelInfo();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    FSkillList SkillList;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    FAttributeSet Attributes;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    FDamageAbsorb DamageAbsorbBonus;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    FDamageAbsorb DamageBonus;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    FDamageResistance DamageResistanceBonus;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    float HealthBonus;    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    float HealthRegenBonus;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    float StaminaBonus; 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    float StaminaRegenBonus;

    //todo add feats
};
