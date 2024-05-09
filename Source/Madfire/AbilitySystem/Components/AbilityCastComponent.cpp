#include "AbilityCastComponent.h"

 UAbilityCastComponent::UAbilityCastComponent(const FObjectInitializer &f) :
	 UAbilityComponent(f)
 {}


void UAbilityCastComponent::ComponentInit(UAbstractAbility *parent)
{}

bool UAbilityCastComponent::ComponentSetup(ACharacter *Component) 
{
	return false;
}