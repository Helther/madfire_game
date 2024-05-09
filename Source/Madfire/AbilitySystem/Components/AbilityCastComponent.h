#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "AbilityComponent.h"
#include "AbilityCastComponent.generated.h"


UCLASS(Abstract, Blueprintable, HideDropdown, EditInlineNew)
class UAbilityCastComponent : public UAbilityComponent
{
    GENERATED_BODY()

public:
    UAbilityCastComponent(const FObjectInitializer &f);
    void ComponentInit(UAbstractAbility *parent) override;
    bool ComponentSetup(ACharacter *Component) override;

    void Interrupt();

};