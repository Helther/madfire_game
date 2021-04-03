// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/AbilityManager.h"


// Sets default values for this component's properties
UAbilityManager::UAbilityManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAbilityManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UAbilityManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

/*
UAbstractAbility *UAbilityManager::CreateSkill(ACharacter *caster,
                                               TSubclassOf<UAbstractAbility> SkillClass)
{
    UAbstractAbility *skill = SkillClass.GetDefaultObject();
    check(skill != nullptr && "Ability Manager: skill is invalid") skill->Caster = caster;
    //for debug only
    skill->CurrentCharges = skill->MaxCharges;
    return skill;
}

AAbilityComponent *UAbilityManager::SpawnComponent(
  AActor *target, FAbilityComponentSpawnParameters SpawnParameters, int impower)
{
    return SpawnParameters.Spawn(target);
}*/
