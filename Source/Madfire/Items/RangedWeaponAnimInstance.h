// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RangedWeaponAnimInstance.generated.h"

/**
 * Basis for anim blueprint with state flags, to drive anim FSM
 */
UCLASS()
class MADFIRE_API URangedWeaponAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    bool IsLoaded;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    bool IsFired;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    bool IsLoading;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float LoadingPlayRate;
};
