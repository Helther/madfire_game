#pragma once

#include "AbilitySystem/AbilityComponents/AbilityTargetSelectionComponent.h"
#include "AbstractAbility.h"
#include "CoreMinimal.h"

#include "ChannelAbility.generated.h"

/*
 */
UCLASS(Blueprintable, BlueprintType, abstract)
class UChannelAbility : public UAbstractAbility
{
    GENERATED_BODY()

public:
    UChannelAbility(const FObjectInitializer &f);

    void AbilityInit() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    UAnimMontage *CastAnimMontage;

    void AbilitySetup(UAbilitySystemComponent *Component) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    TSubclassOf<UAbilityTargetSelectionComponent> TargetSelectionClass;

    UPROPERTY(BlueprintReadOnly, Category = "AbilitySystem")
    UAbilityTargetSelectionComponent *TargetSelection;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AbilitySystem")
    float CastRate = 0.5f;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AbilitySystem")
    void ChannelUpdate();
    void ChannelUpdate_Implementation();

    UFUNCTION()
    void PrecastFinish(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION()
    void CastFinish(UAnimMontage *animMontage, bool bInterrupted);

    bool bIsActivated = false;

    const FName PrecastSectionName = "Precast Section";
    const FName CastLoopSectionName = "Cast Loop Section";
    const FName AftercastSectionName = "Aftercast Section";

    FOnMontageBlendingOutStarted OnPrecastFinish;
    FOnMontageBlendingOutStarted OnCastFinish;
    FOnMontageBlendingOutStarted OnReleaseFinish;

    FTimerHandle FuzeTimerHandle;


protected:
    void OnActivate() override;
    void OnRelease() override;
    void OnCancel() override;


    bool StartAnimation(FName SectionName, FOnMontageBlendingOutStarted Delegate);

/*
public:
    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnEventKeyPressed();
    void OnEventKeyPressed_Implementation();

    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnEventKeyReleased();
    void OnEventKeyReleased_Implementation();

    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void InterruptSkill();
    void InterruptSkill_Implementation();

    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void StopSkill();
    void StopSkill_Implementation();

protected:

        void StartCast(void);

        void StopCast(void);

        void OnSkillNotify(void);

        void OnTick(void);*/

    // UAbstractAbility interface
};
