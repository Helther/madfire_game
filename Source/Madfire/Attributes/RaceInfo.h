#pragma once


#include "../Rules/GameRules.h"
#include "CoreMinimal.h"
#include "Attributes.h"
#include "DamageMap.h"
#include "Skills.h"
#include "RaceInfo.generated.h"



UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class URaceInfo : public UDataAsset
{
    GENERATED_BODY()
public:
    // TODO add damage ress
    //Talents
    UPROPERTY(EditDefaultsOnly)
    FAttributeSet Attributes;
    UPROPERTY(EditDefaultsOnly)
    FSkillList Skills;
    UPROPERTY(EditDefaultsOnly)
    FText RaceName;
    UPROPERTY(EditDefaultsOnly)
    FText Description;
};
