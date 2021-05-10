// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/AbilitySystemComponent.h"


// Sets default values for this component's properties
UAbilitySystemComponent::UAbilitySystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UAbilitySystemComponent::ActivateAbility()
{
    if(CurrentAbility->IsValidLowLevelFast())
    {
        CurrentAbility->ActivateAbility();
    }
}

void UAbilitySystemComponent::ReleaseAbility()
{
    if(CurrentAbility->IsValidLowLevelFast())
    {
        CurrentAbility->ReleaseAbility();
    }
}



// Called when the game starts
void UAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...	
}

void UAbilitySystemComponent::SetCurrentAbility(UAbstractAbility *Ability)
{
    CurrentAbility = Ability;
    if(CurrentAbility)
        CurrentAbility->AbilitySetup(this);
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("AbilitySystem Component: Ability Is Null"));
    }
}

// Called every frame
void UAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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
