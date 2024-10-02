#include "AbilityActionComponent.h"

void UAbilityAction::ExecuteActor(AActor *Actor)
{
	FHitResult result;
	result.Init();
	result.HitObjectHandle = FActorInstanceHandle(Actor);
	Execute(result);
}