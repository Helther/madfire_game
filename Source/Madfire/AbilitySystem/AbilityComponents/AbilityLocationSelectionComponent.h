#pragma once

#include "AbilitySystem/AbilityComponents/AbilityComponent.h"
#include "AbilitySystem/AbilityComponents/AbilityTargetSelectionComponent.h"
#include "AbilityLocationSelectionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLocationChoosed, FVector , Location);

UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class UAbilityLocationSelection : public UAbilityTargetSelection
{
    GENERATED_BODY()

public:
    void ComponentInit(UAbstractAbility *parent) override;
    bool ComponentSetup(UAbilitySystemComponent *Component) override;

    UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "AbilitySystem")
    FLocationChoosed OnLocationChoosed;

};
