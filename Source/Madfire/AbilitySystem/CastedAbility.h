#pragma once

#include "AbilitySystem/AbilityComponents/AbilityTargetSelectionComponent.h"
#include "AbilitySystem/AbilityComponents/AbilityAimComponent.h"
#include "AbilitySystem/AbstractAbility.h"
#include "CastedAbility.generated.h"


UCLASS(Blueprintable, BlueprintType, Abstract)
class UCastedAbility : public UAbstractAbility
{
    GENERATED_BODY()

public:
    UCastedAbility(const FObjectInitializer &f);

    void AbilityInit() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    UAnimMontage *CastAnimMontage;

    void AbilitySetup(UAbilitySystemComponent *Component) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    TSubclassOf<UAbilityTargetSelectionComponent> TargetSelectionClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    TSubclassOf<UAbilityAimComponent> AimAlgorithmClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    UAbilityTargetSelectionComponent *TargetSelectionComp;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    UAbilityComponent *AimComp;

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

    const FName PrecastSectionName = "Precast Section";
    const FName CastLoopSectionName = "Cast Loop Section";
    const FName CastReadySectionName = "Cast Ready Section";
    const FName AftercastSectionName = "Release Section";

    FOnMontageBlendingOutStarted OnPrecastFinish;
    FOnMontageBlendingOutStarted OnCastFinish;
    FOnMontageBlendingOutStarted OnCastReadyFinish;
    FOnMontageBlendingOutStarted OnReleaseFinish;




private:


protected:
    void OnActivate() override;
    void OnRelease() override;
    void OnCancel() override;

    bool StartAnimation(FName SectionName, FOnMontageBlendingOutStarted Delegate);

    bool bCastReady = false;
};


