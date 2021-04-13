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
* Тут я удалил всякие ненужные функции, оставил только интерфейс
* Интерфейс я тоже немного поменял - Убрал эти пустые реализации. Должно сработать
* Список Экшонов в скиле я заменил на список Компонентов (см. SkillComponent)
* Также там внизу есть мэнеджер, который может спавнить Скилы и компоненты к ним.
*/

UCLASS(Blueprintable, Abstract)
class UAbstractAbility : public UObject, public FTickableGameObject
{
    GENERATED_BODY()

public:
    UAbstractAbility(const FObjectInitializer &);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "AbilitySystem")
    void Execute(AActor *target);

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

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AbilitySystem")
    int32 CurrentCharges = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    int32 ChargePerCooldown = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    int32 MaxCharges = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    float CastTime = 1;

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
    void RealeseAbility();

    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    void CancelAbility();

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AbilitySystem")
    bool bIsCooldown = false;

    UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "AbilitySystem")
    FAbilityEvent OnAbilityRelease;

    UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "AbilitySystem")
    FAbilityEvent OnAbilityInterrupt;

    UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "AbilitySystem")
    FAbilityEvent OnAbilityCancel;

    UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "AbilitySystem")
    FAbilityEvent OnAbilityStart;

    UPROPERTY(BlueprintReadOnly, Category = "AbilitySystem")
    UAbilitySystemComponent *AbilitySystemComponent;

    void Tick(float DeltaTime) override;

    bool IsTickable() const override;

    bool IsTickableInEditor() const override;

    bool IsTickableWhenPaused() const override;

    TStatId GetStatId() const override;

    UWorld* GetWorld() const override;

    //Called by AbilitySystemComponent when Ability Selected
    virtual void AbilitySetup(UAbilitySystemComponent *Component);



protected:
    FTimerHandle CastTimer;

    inline void ConsumeRequirements(void)
    {
        for (auto &Requirment : Requirements)
        {
            //Requirment->Consume(Caster);
        }
    }

    void StartCooldown()
    {
        bIsCooldown = true;
        CurrentCooldownTime = 0;
    }

    virtual void OnActivate() {};
    virtual void OnRelease() {};
    virtual void OnCancel() {};

    void AddComponent(UAbilityComponent *Component)
    {
        AbilityComponents.Add(Component);
    }

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



