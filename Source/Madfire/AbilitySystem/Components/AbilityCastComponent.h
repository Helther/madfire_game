#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "AbilityComponent.h"
#include "Engine/MemberReference.h"
#include "AbilityCastComponent.generated.h"


USTRUCT(BlueprintType)
struct FAbilityCastStage
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    UAnimMontage *AnimMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    FName SectionName;

    UPROPERTY(EditDefaultsOnly, meta=(FunctionReference, AllowFunctionLibraries, PrototypeFunction="/Script/Madfire.AbstractAbility.OnEventPrototype", DefaultBindingName="OnAnimStart", DisplayName="OnStart"))
    FMemberReference OnStart;

    UPROPERTY(EditDefaultsOnly, meta=(FunctionReference, AllowFunctionLibraries, PrototypeFunction="/Script/Madfire.AbstractAbility.OnEventPrototype", DefaultBindingName="OnAnimStart", DisplayName="OnEnd"))
    FMemberReference OnEnd;
};


UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class UAbilityCastComponent : public UAbilityComponent
{
    GENERATED_BODY()

public:
    UAbilityCastComponent(const FObjectInitializer &f);
    void ComponentInit(UAbstractAbility *parent) override;
    bool ComponentSetup(ACharacter *Component) override;

    void Interrupt();    

    UFUNCTION(BlueprintCallable)
    void Activate(int32 Stage = 0);

    UFUNCTION(BlueprintCallable)
    void Deactivate();
        
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    float CastTime = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    TArray<FAbilityCastStage> CastStages;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentStage = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    int32 LastStage = 0;

protected:
    /*void AnimEnd(UAnimMontage *animMontage, bool bInterrupted);*/
    void OnAnimBlendoutStart(UAnimMontage *animMontage, bool bInterrupted);

    bool StartAnimation(const FAbilityCastStage &);
    bool StopAnimation(const FAbilityCastStage &);

    void PlayNext();
};