#pragma once


#include "AbilitySystem/AbstractAbility.h"
#include "AbilitySystem/AbilityComponents/AbilityTargetSelectionComponent.h"
#include "InstantAbility.generated.h"


UCLASS(Blueprintable, Abstract)
class UInstantAbility : public UAbstractAbility
{
    GENERATED_BODY()

public:
    void AbilityInit() override;

    void AbilitySetup(UCombatComponent *Component) override;

    UFUNCTION()
    void PrecastFinish(UAnimMontage *animMontage, bool bInterrupted);

    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnCastNotify();

    FOnMontageBlendingOutStarted OnPrecastFinish;

    const FName PrecastSectionName = "Precast Section";

protected:
    void Activate() override;
    void Release() override;
    void Cancel() override;
    void Interrupt() override;

    virtual void CastNotify();
};
