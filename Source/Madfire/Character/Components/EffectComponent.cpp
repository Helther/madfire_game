// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/EffectComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/CharacterBase.h"


// Sets default values for this component's properties
UEffectComponent::UEffectComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

    // ...
}

void UEffectComponent::Append(UEffectBase *effect)
{
    ACharacterBase *owner = Cast<ACharacterBase>(GetOwner());

    if(!owner)
    {
        UE_LOG(LogTemp,Warning, TEXT("UEffectComponent: No Owner"))
        return;
    }

    verify(effect != nullptr && "Effect is not Valid");

    auto containedEffect = Find(effect->EffectID);

    if(containedEffect)
    {
        RemoveParticle(containedEffect->EffectParticle);
        containedEffect->Combine(effect);
        AddParticle(containedEffect->EffectParticle, containedEffect->EffectParticleSocket);
    }

    EffectQueue.AddUnique(effect);
    AddParticle(effect->EffectParticle, containedEffect->EffectParticleSocket);
    effect->OnApplying(owner);
}

void UEffectComponent::Remove(UEffectBase *effect)
{
    ACharacterBase *owner = Cast<ACharacterBase>(GetOwner());

    if(!owner)
    {
        UE_LOG(LogTemp,Warning, TEXT("UEffectComponent: No Owner"))
        return;
    }

    effect->OnEnding(owner);
    effect->ConditionalBeginDestroy();
    effect = nullptr;
    EffectQueue.Remove(effect);
}

UEffectBase *UEffectComponent::Find(FName effectName)
{
    for(auto &effect : EffectQueue)
    {
        if(effect->EffectID == effectName)
            return effect;
    }
    return nullptr;
}


// Called when the game starts
void UEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...	
}


// Called every frame
void UEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    Update(DeltaTime);
    // ...
}

void UEffectComponent::Update(float DeltaTime)
{
    ACharacterBase *owner = Cast<ACharacterBase>(GetOwner());

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
            effect->ConditionalBeginDestroy();
            effect = nullptr;
        }
    }

    EffectQueue.RemoveAll([](UEffectBase* effect) -> bool {return effect == nullptr;});
}

void UEffectComponent::AddParticle(UParticleSystem *particle, FName AttachPoint)
{
    ACharacterBase *owner = Cast<ACharacterBase>(GetOwner());

    //Get Mesh of Characeter
    auto mesh = owner->GetMesh();

    verify(mesh != nullptr && "Character doesn't have a mesh, How this is happen?");

    auto ParComp = UGameplayStatics::SpawnEmitterAttached(particle,
                                                           mesh,
                                                           AttachPoint,
                                                           FVector(),
                                                           FRotator(),
                                                           FVector(),
                                                           EAttachLocation::SnapToTargetIncludingScale,
                                                           true,
                                                           EPSCPoolMethod::AutoRelease,
                                                           true);
    EffectParticles.Emplace(particle, ParComp);

}

void UEffectComponent::RemoveParticle(UParticleSystem *particle)
{
    EffectParticles[particle]->Deactivate();
    EffectParticles.Remove(particle);
}

