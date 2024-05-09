#pragma once

#include "AbilitySystem/Components/AbilityComponent.h"
#include "AbilityAimComponent.generated.h"


UCLASS(Abstract, Blueprintable, BlueprintType)
class UAbilityAimComponent : public UAbilityComponent
{
    GENERATED_BODY()

public:

    // UAbilityComponent interface
public:
    void ComponentInit(UAbstractAbility *parent) override;
    bool ComponentSetup(ACharacter *Component) override;

};
