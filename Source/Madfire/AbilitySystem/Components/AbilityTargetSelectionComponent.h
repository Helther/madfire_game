#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "AbilityComponent.h"
#include "GameFramework/Actor.h"
#include "AbilityTargetSelectionComponent.generated.h"

DECLARE_DELEGATE_ThreeParams(FAbilityExecuteEvent, const TArray<AActor*> &, FVector, FRotator);

UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class UAbilityTargetSelection : public UAbilityComponent
{
    GENERATED_BODY()

public:
    // UAbilityComponent interface
    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    void OnExecute(TArray<AActor*> Targets, FVector InstigatorLocation, FRotator InstigatorRotation);

    FAbilityExecuteEvent OnExecuteEvent;
public:
    void ComponentInit(UAbstractAbility *parent) override;
    bool ComponentSetup(UCombatComponent *Component) override;
};
