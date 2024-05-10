#pragma once


#include "AbilitySystem/AbstractAbility.h"
#include "AbilitySystem/Components/AbilityTargetSelectionComponent.h"
#include "InstantAbility.generated.h"


UCLASS(Blueprintable, Abstract)
class UInstantAbility : public UAbstractAbility
{
    GENERATED_BODY()

public:
    void AbilityInit() override;

    void AbilitySetup(ACharacter *Component) override;

    UFUNCTION()
    void PrecastFinish(UAnimMontage *animMontage, bool bInterrupted);

    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnCastNotify();

    FOnMontageBlendingOutStarted OnPrecastFinish;

    const FName PrecastSectionName = "Precast Section";

protected:
    void Activate_deprecated() override;
    void Release_deprecated() override;
    void Cancel_deprecated() override;
    void Interrupt_deprecated() override;

    virtual void CastNotify();
};
