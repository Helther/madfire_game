// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemLog.h"
#include "AbstractAbility.h"
#include "CoreMinimal.h"
#include "Engine.h"
#include "UObject/NoExportTypes.h"

#include "AbstractAbilitySlot.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, abstract)
class UAbstractAbilitySlot : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Slot")
    ACharacter *Caster;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    UAbstractAbility *AbilityReference;

protected:
    UFUNCTION(BlueprintCallable, Meta = (BlueprintProtected = "true"), Category = "Slot")
    void InitSpell(ACharacter *_caster, TSubclassOf<UAbstractAbility> ability);

    UFUNCTION(BlueprintCallable, Meta = (BlueprintProtected = "true"), Category = "Slot")
    void KeyPressed();

    UFUNCTION(BlueprintCallable, Meta = (BlueprintProtected = "true"), Category = "Slot")
    void KeyReleased();

    UFUNCTION(BlueprintCallable, Meta = (BlueprintProtected = "true"), Category = "Slot")
    void GlobalCooldown();

    UFUNCTION(BlueprintCallable, Meta = (BlueprintProtected = "true"), Category = "Slot")
    float Cooldown();

    UFUNCTION(BlueprintCallable, Meta = (BlueprintProtected = "true"), Category = "Slot")
    bool RestoreCharge();
};
