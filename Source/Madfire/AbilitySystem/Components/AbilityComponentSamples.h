#pragma once


#include "AbilitySystem/Components/AbilityTargetSelectionComponent.h"
#include "AbilitySystem/Components/AbilityProjectile.h"
#include "AbilityComponentSamples.generated.h"



UCLASS(EditInlineNew, Blueprintable)
class UAbilitySelfSelection : public UAbilityTargetSelection
{
    GENERATED_BODY()

protected:
    void ActivateComponent_Implementation() override;
};

UCLASS(EditInlineNew, Blueprintable)
class UAbilityProjectileSpawn : public UAbilityTargetSelection
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FVector SpawnOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    TSubclassOf<AAbstractProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FName SocketName;

protected:
    void ActivateComponent_Implementation() override;

    void UpdateComponent_Implementation() override;
};
