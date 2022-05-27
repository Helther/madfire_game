// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/StatusEffect.h"
#include "Character/CharacterBase.h"




void UStatusEffect::OnUpdate_Implementation(ACharacterBase *Target, float DeltaTime)
{
    CurrentDuration-=DeltaTime;
}

AActor *UStatusEffect::SpawnStatusEffectActor(AActor *Owner, TSubclassOf<AActor> ActorClass, FAbilityActorSpawnParameters param, FName Name)
{
    //TODO: Merge with UAbilityFunctional::SpawnAbilityActor
    check(ActorClass && "Actor Class is Invalid");
    constexpr bool NoCollisionFail = false; //A bool that determines whether a collision test will be performed when spawning the Actor. If true, no collision test will be performed when spawning the Actor regardless of the collision settings of the root component or template Actor.
    constexpr bool RemoteOwned = false; //Is the actor remotely owned. This should only be set true by the package map when it is creating an actor on a client that was replicated from the server.
    constexpr bool DeferConstruction = false; //Only applicable if the Actor is being spawned from a Blueprint.

    if(Owner)
    {
        auto Actor = Owner->GetWorld()->SpawnActor(ActorClass, &param.Location, &param.Rotation);
        Actor->SetOwner(Owner);
        return Actor;
    }
    else
        return nullptr;
}
