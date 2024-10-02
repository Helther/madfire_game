#pragma once

#include "Components/AbilityComponent.h"
#include "Character/AbstractRequirement.h"
#include "Character/Components/MovementComponentBase.h"
#include "Character/Components/Camera/CameraMode.h"
#include "CoreMinimal.h"
#include "Engine.h"
#include "GameFramework/Actor.h"
#include "AbstractAbility.generated.h"

class ACharacter;

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


USTRUCT(BlueprintType)
struct FAbilityInfo : public FTableRowBase
{

    GENERATED_USTRUCT_BODY()

    FAbilityInfo() :
        AbilityIcon(nullptr),
        Description(FText::FromString("Undefined")),
        Name(FText::FromString("Undefined"))
    {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    UTexture2D *AbilityIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    int32 MaxCharges = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    TSubclassOf<class UAbstractAbility> AbilityClass;
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

    UPROPERTY(BlueprintReadOnly, Category = "AbilitySystem")
    bool bIsActive = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    UAnimMontage *CastAnimMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Bitmask, BitmaskEnum = "EMoveAbilities"), Category = "AbilitySystem")
    int32 MoveAbilities = EMoveAbilities::NoMove;

    UPROPERTY(BlueprintReadOnly, Category = "AbilitySystem")
    ACharacter *CharacterOwner;

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

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AbilitySystem")
    bool bIsCooldown = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    FName AbilityName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    TSubclassOf<UCameraMode> AbilityCameraMode;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystem")
    UDataTable *InfoDataTable = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Ability Components")
    TArray<UAbilityComponent*> AbilityComponents; //TODO TSharedPtr

    //Interface
    virtual void AbilityInit();

    //Called by AbilitySystemComponent when Ability Selected
    virtual void AbilitySetup(ACharacter *Character);

    void Tick(float DeltaTime) override;

    bool IsTickable() const override;

    bool IsTickableInEditor() const override;

    bool IsTickableWhenPaused() const override;

    TStatId GetStatId() const override;

    UWorld* GetWorld() const override;

    //Controll interface
    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    void ActivateAbility();

    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    void ReleaseAbility();

    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    void CancelAbility();
    //Blueprint Events
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

    //Helper Functional
    //Blueprint Inteface    
    UFUNCTION(BlueprintPure, Category = "AbilitySystem")
    float GetCooldownTimeRemaining() const;

    UFUNCTION(BlueprintPure, Category = "AbilitySystem")
    const FAbilityInfo &GetInfo() const;

    UFUNCTION(BlueprintCallable, Meta=(DefaultToSelf="Parent", DeterminesOutputType="AbilityClass", DeprecatedFunction), Category = "AbilitySystem")
    static UAbstractAbility *SpawnAbility(UObject *Parent, TSubclassOf<UAbstractAbility> AbilityClass, FName Name = NAME_None);

    UFUNCTION(BlueprintCallable, Meta=(DefaultToSelf="Parent", DeterminesOutputType= "TargetSelectionClass", DeprecatedFunction), Category = "AbilitySystem")
    static UAbilityComponent *CreateComponent(UAbstractAbility *Parent, TSubclassOf<UAbilityComponent> ComponentClass, FName Name = NAME_None);

    UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "ActorClass", DeprecatedFunction), Category = "AbilitySystem")
    static AActor *SpawnAbilityActor(AActor *Owner, TSubclassOf<AActor> ActorClass, FAbilityActorSpawnParameters param, FName Name = NAME_None);

    UFUNCTION(BlueprintPure, Category = "AbilitySystem")
    static const FAbilityInfo &GetAbilityInfo(UDataTable *AbilityInfoDT, FName AblName);

    UFUNCTION(BlueprintPure, Category="Ability Components", meta=(Keywords = "Actor Owning Parent"))
	AActor* GetOwner() const;

    UFUNCTION(BlueprintPure, Category = "Ability Components")
    UAbilityComponent *GetComponentByName(FName Name) const;

    UFUNCTION(BlueprintPure, Category = "Ability Components")
    TArray<UAbilityComponent*> GetComponentsByClass(UClass *Class) const;
       
	template< class T >
	T* GetOwner() const
	{
		return Cast<T>(GetOwner());
	}

    template<typename T>
    TArray<UAbilityComponent*> GetComponentsByClass() const
    {
        return GetComponentsByClass(T::StaticClass());
    }

    UFUNCTION(BlueprintImplementableEvent, Category = "Ability Interface")
    void Created(); //Call when ability manager create this ability 

    UFUNCTION(BlueprintImplementableEvent, Category = "Ability Interface")
    void PostCreated(); //Call when ability manager create this ability 

    UFUNCTION(BlueprintImplementableEvent, Category = "Ability Interface")
    void Selected(); //Call when ability manager select this ability

    UFUNCTION(BlueprintImplementableEvent, Category = "Ability Interface")
    void Unselected(); //Call when ability manager unselect this ability
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Ability Interface")
    void Activate(); //Call when ability manager activate this ability

    UFUNCTION(BlueprintImplementableEvent, Category = "Ability Interface")
    void Interrupt(); //Call when someone interrupt this ability (Can be called by components)

    UFUNCTION(BlueprintCallable,BlueprintImplementableEvent, Category = "Ability Interface") 
    void Release(); //Call when ability manager release this ability

    UFUNCTION(BlueprintImplementableEvent, Category = "Ability Interface")
    void Cancel(); //Call when ability manager interrupts or cancel this ability

    UFUNCTION()
    void OnEventPrototype() {}

protected:
    FTimerHandle CastTimer;

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

    virtual void Activate_deprecated();
    virtual void Release_deprecated();
    virtual void Cancel_deprecated();
    virtual void Interrupt_deprecated();

    void AddComponent(UAbilityComponent *Component)
    {
        check(Component != nullptr);
        AbilityComponents.Add(Component);
    }

    bool StartAnimation(FName SectionName, FOnMontageBlendingOutStarted Delegate);
    bool StopAnimation();

    bool bUserAnimInterrupt = false;

    static constexpr auto DataTablePath = "AbilitySystem/AbilityTable";
            
    // New Ability System
    UAbilityComponent *CreateComponent(UClass *CompClass, FName Name)
    {
        auto ReturnType = UAbilityComponent::StaticClass();
        auto Return = Cast<UAbilityComponent>(CreateDefaultSubobject(Name, ReturnType, CompClass->GetClass(), true, false));
        AbilityComponents.Add(Return);
        return Return;
    }

    template<typename ComponentType>
    UAbilityComponent *CreateComponent(FName Name)
    {
        auto Comp = CreateDefaultSubobject<ComponentType>(Name);
        AbilityCompoentns.Add(Comp);
        return Comp;
    }
};



