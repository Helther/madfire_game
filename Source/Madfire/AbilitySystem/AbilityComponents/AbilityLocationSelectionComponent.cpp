#include "AbilityLocationSelectionComponent.h"
#include "Character/Components/CombatComponent.h"

void UAbilityLocationSelection::ComponentInit(UAbstractAbility *parent)
{
    OnLocationChoosed.AddDynamic(parent,&UAbstractAbility::OnLocationExecute);
    Super::ComponentInit(parent);
}

bool UAbilityLocationSelection::ComponentSetup(UCombatComponent *Component)
{
    return Super::ComponentSetup(Component);
}

