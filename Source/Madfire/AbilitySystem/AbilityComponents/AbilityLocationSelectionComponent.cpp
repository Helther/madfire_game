#include "AbilityLocationSelectionComponent.h"
#include "Character/Components/AbilitySystemComponent.h"

void UAbilityLocationSelection::ComponentInit(UAbstractAbility *parent)
{
    OnLocationChoosed.AddDynamic(parent,&UAbstractAbility::OnLocationExecute);
    Super::ComponentInit(parent);
}

bool UAbilityLocationSelection::ComponentSetup(UAbilitySystemComponent *Component)
{
    return Super::ComponentSetup(Component);
}

