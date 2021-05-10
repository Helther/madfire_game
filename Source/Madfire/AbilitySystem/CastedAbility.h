#pragma once

#include "AbilitySystem/AbilityComponents/AbilityTargetSelectionComponent.h"
#include "AbilitySystem/AbilityComponents/AbilityAimComponent.h"
#include "AbilitySystem/AbstractAbility.h"
#include "CastedAbility.generated.h"


UCLASS(Blueprintable, Abstract)
class UCastedAbility : public UAbstractAbility
{
    GENERATED_BODY()

public:
    UCastedAbility(const FObjectInitializer &f);

    void AbilityInit() override;

    void AbilitySetup(UAbilitySystemComponent *Component) override;

    UPROPERTY(BlueprintReadOnly, Category = "AbilitySystem")
    float CurrentCastTime;

    UPROPERTY(BlueprintReadOnly, Category = "AbilitySystem")
    bool bIsCasted = false;

    void Tick(float DeltaTime) override;

    UFUNCTION()
    void PrecastFinish(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION()
    void CastFinish(UAnimMontage *animMontage, bool bInterrupted);   
    UFUNCTION()
    void CastReadyFinish(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION()
    void ReleaseFinish(UAnimMontage *animMontage, bool bInterrupted);

    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnCastNotify();
    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnReleaseNotify();
    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnAbilityReady();


    const FName PrecastSectionName = "Precast Section";
    const FName CastLoopSectionName = "Cast Loop Section";
    const FName CastReadySectionName = "Cast Ready Section";
    const FName AftercastSectionName = "Release Section";

    FOnMontageBlendingOutStarted OnPrecastFinish;
    FOnMontageBlendingOutStarted OnCastFinish;
    FOnMontageBlendingOutStarted OnCastReadyFinish;
    FOnMontageBlendingOutStarted OnReleaseFinish;

protected:
    void Activate() override;
    void Release() override;
    void Cancel() override;
    void Interrupt() override;

    virtual void CastNotify();
    virtual void AbilityReady();
    virtual void ReleaseNotify();

    bool bCastReady = false;
};


