#pragma once

#include "AbilitySystem/AbstractAbility.h"
#include "AbstractSpell.generated.h"


USTRUCT(BlueprintType)
struct FSpell
{
    GENERATED_BODY()

    FSpell();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    FName AbilityName;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    UDataTable *InfoDataTable = nullptr;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    int32 SpellLevel;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    EMagicSchool MagicSchool;

    const FAbilityInfo &GetAbilityInfo() const;
};

/*
UCLASS()
class USpell : public UObject
{

    GENERATED_BODY()
public:
    USpell();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    FName AbilityName;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    UDataTable *InfoDataTable = nullptr;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    int32 SpellLevel;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    EMagicSchool MagicSchool;
};
*/
