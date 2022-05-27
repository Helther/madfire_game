// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/StatusEffectComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/CharacterBase.h"


// Sets default values for this component's properties
UStatusEffectQueueComponent::UStatusEffectQueueComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

    // ...
}

void UStatusEffectQueueComponent::Append(UStatusEffect *effect)
{
    check(this != nullptr && "Effect Component is Invalid");

    ACharacterBase *owner = Cast<ACharacterBase>(GetOwner());

    if(!owner)
    {
        UE_LOG(LogTemp,Warning, TEXT("UEffectComponent: No Owner"))
        return;
    }

    check(effect != nullptr && "UEffectComponent: Effect is INVALID");

    auto containedEffect = Find(effect->EffectID);

    if(containedEffect)
    {        
        containedEffect->Combine(effect);        
        return;
    }

    EffectQueue.AddUnique(effect);    
    effect->OnApplying(owner);
    OnAppend.Broadcast(effect);
}

void UStatusEffectQueueComponent::Remove(UStatusEffect *effect)
{
    ACharacterBase *owner = Cast<ACharacterBase>(GetOwner());

    if(!owner)
    {
        UE_LOG(LogTemp,Warning, TEXT("UEffectComponent: No Owner"))
        return;
    }

    if(!EffectQueue.Contains(effect))
    {
        UE_LOG(LogTemp,Warning, TEXT("UEffectComponent: No such effect"))
        return;
    }

    EffectQueue.Remove(effect);
    effect->OnEnding(owner);
    OnRemove.Broadcast(effect);
    effect->ConditionalBeginDestroy();
    effect = nullptr;        
}

UStatusEffect *UStatusEffectQueueComponent::Find(FName effectName)
{
    for(auto &effect : EffectQueue)
    {
        if(effect->EffectID == effectName)
            return effect;
    }
    return nullptr;
}


// Called when the game starts
void UStatusEffectQueueComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...	
}


// Called every frame
void UStatusEffectQueueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    Update(DeltaTime);
    // ...
}

void UStatusEffectQueueComponent::Update(float DeltaTime)
{
    ACharacterBase *owner = Cast<ACharacterBase>(GetOwner());
    // TODO apply efefcts to attributes
    if(!owner)
    {
        UE_LOG(LogTemp,Warning, TEXT("UEffectComponent: No Owner"))
        return;
    }


    for(auto &effect : EffectQueue)
    {
        verify(effect != nullptr && "effect is nullptr, How this is happen?");

        effect->OnUpdate(owner,DeltaTime);

        if(effect->CurrentDuration <= 0)
        {
            effect->OnEnding(owner);
            OnRemove.Broadcast(effect);
            effect->ConditionalBeginDestroy();
            effect = nullptr;            
        }
    }

    EffectQueue.RemoveAll([](UStatusEffect* effect) -> bool {return effect == nullptr;});
}
