#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "AbilityComponent.h"
#include "GameFramework/Actor.h"
#include "AbilityTargetSelectionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetChoosed, AActor *, Actor);


UCLASS(Blueprintable, BlueprintType)
class UAbilityTargetSelectionComponent : public UAbilityComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "AbilitySystem")
    FTargetChoosed OnTargetChoosed;
};
