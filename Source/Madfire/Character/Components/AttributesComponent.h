// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"

#include "../../Attributes/Attributes.h"
#include "../../Attributes/ConsumableAttributes.h"
#include "../../Attributes/Damage.h"
#include "../../Attributes/DamageMap.h"
#include "../../Attributes/Skills.h"
#include "../../Attributes/RaceInfo.h"
#include "Attributes/Barrier.h"
#include "Attributes/CharacterLevelInfo.h"
#include "AttributesComponent.generated.h"


constexpr int MaxLevel = 30;

/**
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UAttributeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAttributeComponent(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    URaceInfo *CharacterRace;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "Character Attributes")
    FAttributeSet Attributes; //!< Базовые аттрибуты (сила, ловкость и т.д.)
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "Character Attributes")
    FSkillList Skills;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "Character Attributes")
    FDamageAbsorb DamageAbsorb;       //!< Базовое снижение урона (на число (поглощение))
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "Character Attributes")
    FDamageAbsorb DamageBonus;       //!< Базовая прибавка к урону (на число или проценты?)
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "Character Attributes")
    FDamageResistance DamageResistance;  //!< Базовое снижение урона (в процентах)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    float AttackSpeed;	/*!< в процентах*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    float CastSpeed;	/*!< в процентах*/

    //Primary Attributes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    UDynamicAttribute *Health;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    UDynamicAttribute *Stamina;

    //Experience
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    float CurrentExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    float FreeSkillPoints; //Skill Points givin you on LevelUp

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    float FreeAttributePoints; //Attribute Points givin you on LevelUp

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Attributes")
    UCurveFloat *ExperienceCurve;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Attributes")
    TArray<FCharacterLevelInfo> LevelList;

    UFUNCTION(BlueprintCallable, Category = "Character Attributes")
    void AddExperience(float Amount);

    UFUNCTION(BlueprintPure, Category = "Character Attributes")
    FORCEINLINE int32 GetCharacterLevel() const {return LevelList.Num();}

    UFUNCTION(BlueprintPure, Category = "Character Attributes")
    float GetLevelupRequiredExp() const;

    UFUNCTION(BlueprintPure, Category = "Character Attributes")
    static FORCEINLINE uint8 GetMaxPossibleLevel() { return MaxLevel; }

    UFUNCTION(BlueprintPure, Category = "Character Attributes")
    FORCEINLINE bool CanLevelUp() const { return (CurrentExperience >= GetLevelupRequiredExp()) && (LevelList.Num() < MaxLevel); }

    UFUNCTION(BlueprintCallable, Category = "Character Attributes")
    void LevelUp(const FCharacterLevelInfo &levelInfo); // используется через персонажа для вызова обновы

    void Update();

    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void SetDirty(bool IsDirty = true) { bIsDirty = IsDirty; }
    UFUNCTION(BlueprintPure, Category = "Component Update")
    bool IsUpdateNeeded() const { return bIsDirty; }

    bool SkillCheck(ESkill Skill, int32 value);

#if WITH_EDITOR
    void PostEditChangeProperty(struct FPropertyChangedEvent &e);
#endif

    //For external use only
    void ModifyAttribute(FAttributeSet Attribute);
    void ModifySkills(FSkillList Skills);
    void ModifyDamageAbsorb(FDamageAbsorb DamageAbsorb);
    void ModifyDamageResist(FDamageResistance DamageResist);
    void ModifyDamageBonus(FDamageAbsorb DamageBonus);
    void ModifyHealthBonus(float HPBonus);
    void ModifyHealthRegen(float HPRegen);
    void ModifyStaminaBonus(float STBonus);
    void ModifyStaminaRegen(float STRegen);


    UPROPERTY(BlueprintReadWrite, Category = "Character Attributes")
    FAttributeSet BaseAttribute;


    FAttributeSet BonusAttribute;    
    FSkillList BonusSkills;    
    FDamageAbsorb BonusDamageAbsorb;    
    FDamageAbsorb BonusDamageBonus;    
    FDamageResistance BonusDamageResistance;    
    float BonusHealth = 0.f;    
    float BonusHealthRegen = 1.f;
    float BonusStamina = 0.f;    
    float BonusStaminaRegen = 1.f;

    void UpdateBaseAttribute();
    void UpdateAttribute();
    void UpdateSkills();
    void UpdateDamageAbsorb();
    void UpdateDamageResist();
    void UpdateDamageBonus();
    void UpdateHealth();
    void UpdateHealthRegen();
    void UpdateStamina();
    void UpdateStaminaRegen();


    //Returns Remount HP
    float TakeDamage(float DamageAmount, UDamageType *DamageType, AActor *DamageCauser);

    UPROPERTY(EditAnywhere)
    TArray<UBarrier*> BarrierList;

    const class ACharacterBase *GetCharacterOwner() const;

private:
    bool bIsDirty = false;



    // UActorComponent interface
public:
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    // Called when the game starts
    virtual void BeginPlay() override;
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};

