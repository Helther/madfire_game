#pragma once

#include "AbilityComponents/AbilityComponent.h"
#include "AbilitySystemLog.h"
#include "Character/AbstractRequirement.h"
#include "Character/Components/MovementComponentBase.h"
#include "CoreMinimal.h"
#include "Engine.h"
#include "GameFramework/Actor.h"
#include "AbstractAbility.generated.h"


class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityEvent, UAbstractAbility*, Ability);

/*
* Тут я удалил всякие ненужные функции
* Интерфейс я тоже много поменял Должно сработать
*/

UCLASS(BlueprintType, Blueprintable, Abstract)
class UAbstractAbility : public UObject, public FTickableGameObject
{
    friend class UAbilityComponent;

    GENERATED_BODY()

public:
    UAbstractAbility(const FObjectInitializer &);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    UAnimMontage *CastAnimMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Bitmask, BitmaskEnum = "EMoveAbilities"), Category = "AbilitySystem")
    int32 MoveAbilities = EMoveAbilities::NoMove;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "AbilitySystem")
    TArray<UAbilityRequirement *> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    TArray<UAbilityComponent *> AbilityComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    float CooldownTime = 3;

    UPROPERTY(BlueprintReadOnly, Category = "AbilitySystem")
    float CurrentCooldownTime = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    float StaminaCost = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    float CastTime = 1;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AbilitySystem")
    int32 CurrentCharges = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    int32 ChargePerCooldown = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    int32 MaxCharges = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    UTexture2D *AbilityIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FText SkillName;

    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    float GetCooldownTimeRemaining() const;

    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    void ActivateAbility();

    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    void ReleaseAbility();

    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    void CancelAbility();

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AbilitySystem")
    bool bIsCooldown = false;

    UPROPERTY(BlueprintReadOnly, Category = "AbilitySystem")
    UAbilitySystemComponent *AbilitySystemComponent;

    //Interface
    virtual void AbilityInit();

    //Called by AbilitySystemComponent when Ability Selected
    virtual void AbilitySetup(UAbilitySystemComponent *Component);

    void Tick(float DeltaTime) override;

    bool IsTickable() const override;

    bool IsTickableInEditor() const override;

    bool IsTickableWhenPaused() const override;

    TStatId GetStatId() const override;

    UWorld* GetWorld() const override;

    //Blueprint Inteface
    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnAbilityInit();

    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnAbilityInterrupt();

    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnAbilityStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnAbilityEnd();

    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnAbilityCancel();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnTargetExecute(AActor *target);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnLocationExecute(FVector Location);

protected:
    FTimerHandle CastTimer;

    inline void ConsumeRequirements(void)
    {
        for (auto &Requirment : Requirements)
        {
            //Requirment->Consume(Caster);
        }
    }

    void StopCooldown()
    {
        bIsCooldown = false;
        CurrentCooldownTime = 0;
    }

    void StartCooldown()
    {
        bIsCooldown = true;
        CurrentCooldownTime = 0;
    }

    virtual void Activate();
    virtual void Release();
    virtual void Cancel();
    virtual void Interrupt();

    void AddComponent(UAbilityComponent *Component)
    {
        check(Component != nullptr);
        AbilityComponents.Add(Component);
    }

    bool StartAnimation(FName SectionName, FOnMontageBlendingOutStarted Delegate);
    bool StopAnimation();

    /*
    inline bool isValid(void)
    {
        check(Caster != nullptr && "Skill: caster not found");
        check(GlobalCoolDownTime > 0.0f && "Skill: cooldown time is not valid");
        check(CastTime > 0 && "Skill: charge cooldown time is not valid");
        check(MaxCharges > 0 && "Skill: max charge count is not valid");


        if (isCasting)
        {
            UE_LOG(AbilitySystemLog, Warning, TEXT("Skill: is casting now"));
            return false;
        }

        if (onCooldown)
        {
            UE_LOG(AbilitySystemLog, Warning, TEXT("Skill: is on cooldown now"));
            return false;
        }

        if (SkillComponents.Num() == 0)
        {
            UE_LOG(AbilitySystemLog, Error, TEXT("Skill: component list is empty"));
            return false;
        }

        return true;
    }*/
};



