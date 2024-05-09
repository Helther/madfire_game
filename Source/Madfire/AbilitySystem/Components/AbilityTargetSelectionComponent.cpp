#include "AbilityTargetSelectionComponent.h"
#include "AbilitySystem/AbstractAbility.h"

void UAbilityTargetSelection::ComponentInit(UAbstractAbility *parent)
{
    Super::ComponentInit(parent);
    OnExecuteEvent.BindUObject(parent, &UAbstractAbility::OnExecute);
}

bool UAbilityTargetSelection::ComponentSetup(ACharacter *Component)
{
    return Super::ComponentSetup(Component);
}

void UAbilityTargetSelection::OnExecute(TArray<AActor*> Targets, FVector InstigatorLocation, FRotator InstigatorRotation)
{
    OnExecuteEvent.ExecuteIfBound(Targets,InstigatorLocation,InstigatorRotation);
}
