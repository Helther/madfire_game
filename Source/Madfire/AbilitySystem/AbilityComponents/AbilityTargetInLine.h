// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/AbilityComponents/AbilityTargetSelectionComponent.h"
#include "CoreMinimal.h"
#include "TraceFunctional/TraceFunctional.h"
#include "AbilityTargetInLine.generated.h"



UCLASS(Blueprintable, BlueprintType)
class UAbilityTargetInLine : public UAbilityTargetSelectionComponent
{
    GENERATED_BODY()

public:
    UAbilityTargetInLine();
    //virtual void BeginPlay() override;
    //virtual void Tick(float DeltaSeconds) override;

    bool ComponentSetup_Implementation(UAbilitySystemComponent *Component);
    void Activate_Implementation();

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
