#include "AbilityFunctional.h"



UAbstractAbility *UAbilityFunctional::SpawnAbility(UObject *Parent, TSubclassOf<UAbstractAbility> AbilityClass, FName Name)
{
    auto NewAbility = NewObject<UAbstractAbility>(Parent, AbilityClass, Name);
    if(!NewAbility)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability Functional: Can't create ability"));
        return nullptr;
    }
    NewAbility->AbilityInit();
    return NewAbility;
}

UAbilityComponent *UAbilityFunctional::CreateComponent(UAbstractAbility *Parent, TSubclassOf<UAbilityComponent> ComponentClass, FName Name)
{
    auto Comp = NewObject<UAbilityComponent>(Parent, ComponentClass, Name);
    if(!Comp)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability Functional: Can't create Target Component"));
        return nullptr;
    }
    Comp->ComponentInit(Parent);

    return Comp;
}

AActor *UAbilityFunctional::SpawnAbilityActor(AActor *Owner, TSubclassOf<AActor> ActorClass, FAbilityActorSpawnParameters param, FName Name)
{
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
