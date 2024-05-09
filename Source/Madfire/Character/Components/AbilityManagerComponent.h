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

    UFUNCTION(BlueprintCallable, Category = "Ability System")
    void SetCurrentAbility(UAbstractAbility *Ability);

    UFUNCTION(BlueprintCallable, Category = "Ability System")
    void ActivateAbility();

    UFUNCTION(BlueprintCallable, Category = "Ability System")
    void ReleaseAbility();

    UFUNCTION(BlueprintCallable, Category = "Ability System")
    bool IsUsingAbility() const;

    void OnUseAbility(bool Started);

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "")
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

    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "")
    int32 MaxLevel;
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "")
    TArray<FLevelSpell> LevelSpellList;
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "")
    TArray<FSpell> SpellList;
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "")
    TMap<FName,UAbstractAbility*> SpellAbilityList;

    bool AbilityPressed = false;
    
    FTimerHandle GlobalCooldownTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
    TSoftObjectPtr<UAbstractAbility> CurrentAbility;

    bool IsAbilityPressed() const;

    bool IsOnGlobalCooldown() const;

    void SetGlobalCooldown();


    //TODO: Update Delegate For Ability panel
    void CalculateMaxCastingSpeed(float SpeedModifier);

    // значение относительно нормы
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    float CastSpeed = 1.f;

    //Max move speed during ability cast
    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Ability System")
    float MaxSpeedWhileCasting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    float GlobalCooldownTime = 1;

    ACharacterCreature *Owner = nullptr;

protected:
    void BeginPlay() override;
};
