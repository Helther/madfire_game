#pragma once

#include "AbilitySystem/Components/AbilityComponent.h"
#include "Character/Components/Camera/CameraMode.h"
#include "AbilityAimComponent.generated.h"


UCLASS(Abstract, Blueprintable, BlueprintType)
class UAbilityAimComponent : public UAbilityComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    TSubclassOf<UCameraMode> AbilityCameraMode;

    // UAbilityComponent interface
public:
    void ComponentInit(UAbstractAbility *parent) override;
    bool ComponentSetup(ACharacter *Component) override;

};
