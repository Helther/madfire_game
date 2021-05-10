#pragma once

#include "AbilitySystem/CastedAbility.h"
#include "AbilitySystem/ChannelAbility.h"
#include "AbilitySystem/InstantAbility.h"
#include "AbilitySystem/AbilityComponents/AbilityAimComponent.h"
#include "AbilitySystem/AbilityComponents/AbilityTargetSelectionComponent.h"
#include "AbilitySystem/AbilityComponents/AbilityActorSelectionComponent.h"
#include "AbilitySystem/AbilityComponents/AbilityLocationSelectionComponent.h"
#include "AbilitySystem/AbilityComponents/AbilitySoundComponent.h"


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
    UAbilityActorSelection *TargetComponent;

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
    UAbilityActorSelection *TargetComponent;

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    UAbilityAimComponent *AimingComponent;

protected:
    void CastNotify() override;
    void Release() override;
    void ReleaseNotify() override;

};


UCLASS(Blueprintable)
class UChannelAbility_Sample : public UChannelAbility
{
    GENERATED_BODY()

public:
    void AbilityInit() override;

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    UAbilityActorSelection *TargetComponent;

    void ChannelUpdate();

protected:
    void Release() override;
    void CastNotify() override;
};


