// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/AbilityComponent.h"
#include "CoreMinimal.h"
#include "TraceFunctional/TraceFunctional.h"

#include "BoxTrace.generated.h"

/*
 * 
 */

UCLASS()
class ABoxTrace : public AAbilityComponent
{
    GENERATED_BODY()

public:
    ABoxTrace();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Spell System")
    //bool ComponentSetup(ACharacter *Parent, int32 ActionParameter);

    bool ComponentSetup_Implementation(ACharacter *Parent, int32 ActionParameter);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<UAbilityAction>> ActionList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector BoxHalfExtent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsSelfApply = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsMultipleTrace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTraceParams TraceParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTraceSpawnParams SpawnParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TEnumAsByte<ECollisionChannel> CollisionChannel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool ReturnPhysMat = false;
};
