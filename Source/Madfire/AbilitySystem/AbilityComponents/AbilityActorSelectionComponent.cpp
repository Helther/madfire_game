#include "AbilityActorSelectionComponent.h"
#include "Character/Components/AbilitySystemComponent.h"

void UAbilityActorSelection::ComponentInit(UAbstractAbility *parent)
{
    OnTargetChoosed.AddDynamic(parent,&UAbstractAbility::OnTargetExecute);
    Super::ComponentInit(parent);
}

bool UAbilityActorSelection::ComponentSetup(UAbilitySystemComponent *Component)
{
    return Super::ComponentSetup(Component);
}

