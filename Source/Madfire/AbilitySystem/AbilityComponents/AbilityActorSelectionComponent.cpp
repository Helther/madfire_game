#include "AbilityActorSelectionComponent.h"
#include "Character/Components/CombatComponent.h"

void UAbilityActorSelection::ComponentInit(UAbstractAbility *parent)
{
    OnTargetChoosed.AddDynamic(parent,&UAbstractAbility::OnTargetExecute);
    Super::ComponentInit(parent);
}

bool UAbilityActorSelection::ComponentSetup(UCombatComponent *Component)
{
    return Super::ComponentSetup(Component);
}

