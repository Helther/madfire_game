// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem/AbstractAbility.h"
#include "AbilitySystemComponent.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UAbilitySystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
    UAbilitySystemComponent();

    UFUNCTION(BlueprintCallable, Category = "Character Ability")
    void SetCurrentAbility(UAbstractAbility *Ability);

    UFUNCTION(BlueprintCallable, Category = "Character Ability")
    void ActivateAbility();

    UFUNCTION(BlueprintCallable, Category = "Character Ability")
    void ReleaseAbility();

    UFUNCTION(BlueprintCallable, Category = "Character Ability")
    void AddAbility(UAbstractAbility *Ability);

    UFUNCTION(BlueprintCallable, Category = "Character Ability")
    void RemoveAbility(UAbstractAbility *Ability);

    UPROPERTY(BlueprintReadOnly, Category = "Character Ability")
    UAbstractAbility *CurrentAbility;

    UPROPERTY(BlueprintReadOnly, Category = "Character Ability")
    TSet<UAbstractAbility*> AbilityList;

    UFUNCTION(BlueprintCallable, Category = "Characeter Ability")
    void StateChanged(/*TChange*/);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
    float GlobalCooldownTime = 1;

private:
    bool CheckState();
};



/*
 * Вот небольшой манагер для скилов
UCLASS()
class UAbilityManager : public UActorComponent
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure, meta = (DisplayName = "Create Skill From Class", CompactNodeTitle = "Create Skill", Keywords = "new create skill"), Category = "Spell System")
    static UAbstractAbility *CreateSkill(ACharacter *caster, TSubclassOf<UAbstractAbility> SkillClass);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Create Skill From Class", CompactNodeTitle = "Create Skill", Keywords = "new create skill"), Category = "Spell System")
    static AAbilityComponent *SpawnComponent(AActor *target, FAbilityComponentSpawnParameters SpawnParameters, int impower);
};*/
