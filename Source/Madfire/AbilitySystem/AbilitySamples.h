#pragma once

#include "AbilitySystem/CastedAbility.h"
#include "AbilitySystem/ChannelAbility.h"
#include "AbilitySystem/InstantAbility.h"
#include "AbilitySystem/Components/AbilityAimComponent.h"
#include "AbilitySystem/Components/AbilityTargetSelectionComponent.h"
#include "AbilitySystem/Components/AbilitySoundComponent.h"


#include "AbilitySamples.generated.h"


UCLASS(Blueprintable)
class UInstantAbility_Sample : public UInstantAbility
{
    GENERATED_BODY()

public:
    void AbilityInit() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    USoundCue *AbilitySound;

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    UAbilitySoundComponent *SoundComponent;

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    UAbilityTargetSelection *TargetComponent;

protected:
    void CastNotify() override;
};


UCLASS(Blueprintable)
class UCastedAbility_Sample : public UCastedAbility
{
    GENERATED_BODY()

public:
    void AbilityInit() override;

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    UAbilityTargetSelection *TargetComponent;

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    UAbilityAimComponent *AimingComponent;

protected:
    void CastNotify() override;
    void Release_deprecated() override;
    void ReleaseNotify() override;
    void Interrupt_deprecated() override; // NOTE don't forget to deactivate component when interrupted
};


UCLASS(Blueprintable)
class UChannelAbility_Sample : public UChannelAbility
{
    GENERATED_BODY()

public:
    void AbilityInit() override;

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    UAbilityTargetSelection *TargetComponent;

    void ChannelUpdate();

protected:
    void Release_deprecated() override;
    void CastNotify() override;
    void Interrupt_deprecated() override;
    void Cancel_deprecated() override;
};


