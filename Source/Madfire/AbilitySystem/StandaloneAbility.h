// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/AbstractAbility.h"
#include "CoreMinimal.h"
#include "StandaloneAbility.generated.h"



UCLASS(Blueprintable, BlueprintType, abstract)
class UStandaloneAbility : public UAbstractAbility
{
    GENERATED_BODY()
/**
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
    bool isImpowerable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
    float MaximumPressureTime = 1.0f;

    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnEventKeyPressed();
    void OnEventKeyPressed_Implementation();

    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnEventKeyReleased();
    void OnEventKeyReleased_Implementation();

    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void InterruptSkill();
    void InterruptSkill_Implementation();

    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void StopSkill();
    void StopSkill_Implementation();

protected:
    float currentPressureTime = 0.0f;
    int impower               = 0;    // from 0 to 100

    void StartCast(void);

    void StopCast(void);

    void OnSkillNotify(void);

    void OnTick(void); */
};

