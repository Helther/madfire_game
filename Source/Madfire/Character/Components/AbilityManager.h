// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem/AbstractAbility.h"
#include "AbilityManager.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MADFIRE_API UAbilityManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAbilityManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	




};



/*
 *
 * Вот небольшой манагер для скилов
UCLASS()
class UAbilityManager : public UActorComponent
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure,
              meta     = (DisplayName      = "Create Skill From Class",
                      CompactNodeTitle = "Create Skill", Keywords = "new create skill"),
              Category = "Spell System")
    static UAbstractAbility *CreateSkill(ACharacter *caster,
                                         TSubclassOf<UAbstractAbility> SkillClass);

    UFUNCTION(BlueprintPure,
              meta     = (DisplayName      = "Create Skill From Class",
                      CompactNodeTitle = "Create Skill", Keywords = "new create skill"),
              Category = "Spell System")
    static AAbilityComponent *
      SpawnComponent(AActor *target, FAbilityComponentSpawnParameters SpawnParameters,
                     int impower);
};*/
