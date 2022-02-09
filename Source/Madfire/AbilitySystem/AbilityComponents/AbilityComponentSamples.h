#pragma once

#include "AbilitySystem/AbilityComponents/AbilityActorSelectionComponent.h"
#include "AbilitySystem/AbilityComponents/Projectile.h"
#include "AbilityComponentSamples.generated.h"



UCLASS(EditInlineNew, Blueprintable)
class UAbilitySelfSelection : public UAbilityActorSelection
{
    GENERATED_BODY()

protected:
    void ActivateComponent_Implementation() override;
};

UCLASS(EditInlineNew, Blueprintable)
class UAbilityProjectileSpawn : public UAbilityActorSelection
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    float SpawnOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    TSubclassOf<AAbstractProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FName SocketName;

protected:
    void ActivateComponent_Implementation() override;
};
