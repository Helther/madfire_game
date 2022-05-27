#pragma once

#include "AbilitySystem/Components/AbilityTargetSelectionComponent.h"
#include "AbstractAbility.h"
#include "CoreMinimal.h"

#include "ChannelAbility.generated.h"

/*
 */
UCLASS(Blueprintable, abstract)
class UChannelAbility : public UAbstractAbility
{
    GENERATED_BODY()

public:
    UChannelAbility(const FObjectInitializer &f);

    void AbilityInit() override;

    void AbilitySetup(UCombatComponent *Component) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AbilitySystem")
    float CastRate = 0.5f;

    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnChannelUpdate();

    UFUNCTION()
    void PrecastFinish(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION()
    void CastFinish(UAnimMontage *animMontage, bool bInterrupted);

    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnCastNotify();

    bool bIsActivated = false;

    const FName PrecastSectionName = "Precast Section";
    const FName CastLoopSectionName = "Cast Loop Section";
    const FName AftercastSectionName = "Aftercast Section";

    FOnMontageBlendingOutStarted OnPrecastFinish;
    FOnMontageBlendingOutStarted OnCastFinish;
    FOnMontageBlendingOutStarted OnReleaseFinish;

    FTimerHandle FuzeTimerHandle;


protected:
    void Activate() override;
    void Release() override;
    void Cancel() override;
    void Interrupt() override;

    virtual void CastNotify();
    virtual void ChannelUpdate();

private:
    void AbilityEnd();
};
