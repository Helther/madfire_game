#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "AbilityComponent.h"
#include "Engine/MemberReference.h"
#include "AbilityCastComponent.generated.h"

UENUM(BlueprintType)
enum class ECastAnimStatus : uint8
{
    Proceed,
    Interrupt,
    Cancel
};

UENUM(BlueprintType, Blueprintable)
enum class ECastStageType
{
    NotLoop,
    Loop,
    NotInterruptable
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FCastAnimationEvent, int32, ID, FName, Name);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FCastEndAnimationEvent, int32, ID, FName, Name, ECastAnimStatus, Status);

USTRUCT(Blueprintable, BlueprintType)
struct FAbilityCastStage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    UAnimMontage *AnimMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FName SectionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    float CastTime = 0.f; //TODO: Cast Rate for section

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    bool ProceedStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    bool IsLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    bool IsTimeBased = false;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    //ECastStageType StageType;
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class UAbilityCastComponent : public UAbilityComponent
{
    GENERATED_BODY()

public:
    UAbilityCastComponent(const FObjectInitializer &f);
    void ComponentInit(UAbstractAbility *parent) override;
    bool ComponentSetup(ACharacter *Component) override;

    void Interrupt();    
    void Cancel();

    UFUNCTION(BlueprintCallable)
    void Activate(int32 Stage = 0);

    UFUNCTION(BlueprintCallable)
    void Deactivate();

    UFUNCTION(BlueprintCallable)
    void AddCastStage(FAbilityCastStage Stage, int32 index);

    UFUNCTION(BlueprintCallable)
    void PlayNext();

    UFUNCTION(BlueprintCallable)
    void Continue();

    UFUNCTION(BlueprintPure)
    FName GetCurrentStateName() const;

    UFUNCTION(BlueprintPure)
    int32 GetCurrentStateID() const;    

    UFUNCTION(BlueprintPure)
    float GetCurrentMaxCastTime() const;
            
    UPROPERTY(BlueprintReadOnly, Category = "AbilitySystem")
    float CurrentCastTime = 0;

    UPROPERTY(BlueprintReadOnly, Category = "AbilitySystem")
    TArray<FAbilityCastStage> CastStages;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentStage = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    int32 LastStage = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(IsBindableEvent=true))
    FCastAnimationEvent OnStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(IsBindableEvent=true))
    FCastAnimationEvent OnEnd;

    void Tick_Implementation(float Delta) override;
        
protected:
    /*void AnimEnd(UAnimMontage *animMontage, bool bInterrupted);*/
    void OnAnimBlendoutStart(UAnimMontage *animMontage, bool bInterrupted);

    bool StartAnimation(const FAbilityCastStage &);
    bool StopAnimation(const FAbilityCastStage &);

    bool bUserIterrupt = false;
    bool bContinue = false;
};

