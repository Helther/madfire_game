#pragma once

#include "Components/AbilityComponent.h"
#include "Character/AbstractRequirement.h"
#include "Character/Components/MovementComponentBase.h"
#include "Character/Components/Camera/CameraMode.h"
#include "CoreMinimal.h"
#include "Engine.h"
#include "GameFramework/Actor.h"
#include "AbstractAbility.generated.h"

class UCombatComponent;

DECLARE_LOG_CATEGORY_EXTERN(AbilitySystemLog, Log, All);

USTRUCT(BlueprintType)
struct FAbilityActorSpawnParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FVector Location;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FRotator Rotation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FVector Scale;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    class APawn *Instigator = nullptr; //
};

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

    bool bIsActive = false;

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
    UCombatComponent *CombatComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    TSubclassOf<UCameraMode> AbilityCameraMode;

    //Interface
    virtual void AbilityInit();

    //Called by AbilitySystemComponent when Ability Selected
    virtual void AbilitySetup(UCombatComponent *Component);

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

    UFUNCTION(BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnExecute(const TArray<AActor*> &Target, FVector InstigatorLocation, FRotator InstigatorRotation);



    //Functional
    UFUNCTION(BlueprintCallable, Meta=(DefaultToSelf="Parent", DeterminesOutputType="AbilityClass"), Category = "AbilitySystem")
    static UAbstractAbility *SpawnAbility(UObject *Parent, TSubclassOf<UAbstractAbility> AbilityClass, FName Name = NAME_None);

    UFUNCTION(BlueprintCallable, Meta=(DefaultToSelf="Parent", DeterminesOutputType= "TargetSelectionClass"), Category = "AbilitySystem")
    static UAbilityComponent *CreateComponent(UAbstractAbility *Parent, TSubclassOf<UAbilityComponent> ComponentClass, FName Name = NAME_None);

    UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "ActorClass"), Category = "AbilitySystem")
    static AActor *SpawnAbilityActor(AActor *Owner, TSubclassOf<AActor> ActorClass, FAbilityActorSpawnParameters param, FName Name = NAME_None);

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

    bool bUserAnimInterrupt = false;
};



