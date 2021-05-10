#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "AbilityComponent.h"
#include "GameFramework/Actor.h"
#include "AbilityTargetSelectionComponent.generated.h"


UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class UAbilityTargetSelection : public UAbilityComponent
{
    GENERATED_BODY()

public:
    // UAbilityComponent interface
public:
    void ComponentInit(UAbstractAbility *parent) override;
    bool ComponentSetup(UAbilitySystemComponent *Component) override;

};
