#pragma once

#include "AbilitySystem/AbstractAbility.h"
#include "AbstractSpell.generated.h"


UCLASS()
class USpell : public UClass
{

    GENERATED_BODY()
public:


    UPROPERTY()
    FText Description;
    UPROPERTY()
    FText Name;
    UPROPERTY()
    int32 SpellLevel;
    //UPROPERTY()

    UPROPERTY()
    TSubclassOf<UAbstractAbility> Ability;

private:

};
