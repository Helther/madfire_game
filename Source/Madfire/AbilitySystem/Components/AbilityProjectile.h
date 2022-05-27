// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Components/AbilityComponent.h"
#include "AbilitySystem/Components/AbilityTargetSelectionComponent.h"
#include "Classes/Components/CapsuleComponent.h"
#include "Classes/Components/SphereComponent.h"
#include "Classes/GameFramework/ProjectileMovementComponent.h"
#include "CoreMinimal.h"

#include "AbilityProjectile.generated.h"


UCLASS(Blueprintable, BlueprintType)
class AAbstractProjectile : public AActor
{
    GENERATED_BODY()

public:
    AAbstractProjectile();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Collision")
    USphereComponent *CollisionComp;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Movement")
    UProjectileMovementComponent *MovementComponent;

    UFUNCTION(BlueprintCallable, Category = "Ability System")
    void Execute(const TArray<AActor*> &Actors, FVector Location, FRotator Rotation);

    FAbilityExecuteEvent OnExecuteEvent;
};
