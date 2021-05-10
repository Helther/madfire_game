#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "AbilityComponent.h"
#include "GameFramework/Actor.h"
#include "AbilitySystem/AbilityComponents/AbilityTargetSelectionComponent.h"
#include "AbilityActorSelectionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetChoosed, AActor *, Actor);


UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class UAbilityActorSelection : public UAbilityTargetSelection
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "AbilitySystem")
    FTargetChoosed OnTargetChoosed;

    // UAbilityComponent interface
public:
    void ComponentInit(UAbstractAbility *parent) override;
    bool ComponentSetup(UAbilitySystemComponent *Component) override;

};
