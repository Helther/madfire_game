// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/AbilityComponents/AbilityComponent.h"
#include "AbilitySystem/AbilityComponents/AbilityTargetSelectionComponent.h"
#include "Classes/Components/CapsuleComponent.h"
#include "Classes/Components/SphereComponent.h"
#include "Classes/GameFramework/ProjectileMovementComponent.h"
#include "CoreMinimal.h"

#include "Projectile.generated.h"



UENUM(BlueprintType)
enum class EProjectileDestroyPolicy : uint8
{
    OnHit,
    OnTargetHit,
    Lifespawn
};
ENUM_CLASS_FLAGS(EProjectileDestroyPolicy);

UCLASS(Blueprintable, BlueprintType)
class AAbstractProjectile : public AActor
{
    GENERATED_BODY()

public:
    AAbstractProjectile();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "EProjectileDestroyPolicy"), Category = "Projectile")
    uint8 DestroyPolicy;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Collision")
    USphereComponent *CollisionComp;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Movement")
    UProjectileMovementComponent *MovementComponent;

    UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Projectile")
    FTargetChoosed OnTargetCoosed;

    UFUNCTION()
    void Hit(UPrimitiveComponent *HitComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit);

    UFUNCTION()
    void Overlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

    UFUNCTION()
    void EndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);
};
