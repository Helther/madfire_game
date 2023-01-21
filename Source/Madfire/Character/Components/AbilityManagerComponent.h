#pragma once

#include "AbilitySystem/Spells/AbstractSpell.h"
#include "Components/ActorComponent.h"
#include "AbilityManagerComponent.generated.h"



USTRUCT(BlueprintType)
struct FLevelSpell
{
    GENERATED_BODY()


    UPROPERTY(BlueprintReadWrite)
    bool isBlocked;
    UPROPERTY(BlueprintReadWrite)
    TArray<FSpell> SpellList;
    UPROPERTY(BlueprintReadWrite)
    int32 MaxSpells;
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UAbilityManagerComponent : public UActorComponent
{
    GENERATED_BODY()


public:
    UAbilityManagerComponent(const class FObjectInitializer &ObjectInitializer);


    UFUNCTION(BlueprintCallable, Category = "")
    void Update();

    UFUNCTION(BlueprintCallable, Category = "")
    UAbstractAbility *GetAbility(FName AbilityName);

    UPROPERTY(BlueprintReadOnly, Category = "")
    TMap<FName,UAbstractAbility*> AbilityList;

    UPROPERTY()
    TMap<FName,UAbstractAbility*> ExtraAbilityList;

    UPROPERTY()
    TMap<FName,UAbstractAbility*> EquipAbilityList;

    UPROPERTY()
    TMap<FName,UAbstractAbility*> TalentAbilityList;


    //Spell Book
    UFUNCTION(BlueprintCallable, Category = "")
    void UpdateSpellBook();

    UPROPERTY(BlueprintReadWrite, Category = "")
    int32 MaxLevel;
    UPROPERTY(BlueprintReadWrite, Category = "")
    TArray<FLevelSpell> LevelSpellList;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
    TArray<FSpell> SpellList;
    UPROPERTY(BlueprintReadWrite, Category = "")
    TMap<FName,UAbstractAbility*> SpellAbilityList;
};
