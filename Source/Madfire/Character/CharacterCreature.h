#pragma once

#include "CharacterBase.h"
#include "Animation/AnimMontage.h"
#include "Components/CombatComponent.h"
#include "Components/EquipmentComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/MovementComponentBase.h"
#include "Components/Camera/CameraMode.h"
#include "Components/AbilityManagerComponent.h"
#include "CharacterCreature.generated.h"

UENUM(BlueprintType)
// Character Control Bit mask with values as priority order (the higher the better)
enum ECCState
{
    NoCC = 0,
    Sleep,
    Charm,
    LossOfBalance,
    Stupor,
    Fear,
    Rage,
    MindControl,
    Stun,
    KnockDown,
    Stasis,
    NumOfModes
};


// Класс, отвечающий за управление состоянием контроля над персом и для слежения степени станлока определенных режимов контроля
USTRUCT(BlueprintType)
struct FCCStateManager
{
    GENERATED_BODY()

    FCCStateManager();

    FORCEINLINE void Init(ACharacterCreature* InCharacter) { Character = InCharacter; }

    static int GetHighestPriorityCC(int bitmask);

    FORCEINLINE bool HasCC() const { return CurrentCCState != ECCState::NoCC; }
    FORCEINLINE bool IsInCCMode(ECCState Mode) const { return CurrentCCState & (1u << Mode); }
    FORCEINLINE bool IsInCCSubMode(unsigned int SubMode) const { return CurrentCCSubState & (1u << SubMode); }
    // sets flag, starts cc timer and stunLock timer, time in seconds (time null means anim driven time)
    void SetCCMode(ECCState Mode, unsigned int SubMode, float *Time = nullptr);
    void ResetCCMode(ECCState Mode);
    FORCEINLINE int CurrentCCPriority() const { return GetHighestPriorityCC(CurrentCCState); }
    FORCEINLINE int CurrentCCSubPriority() const { return GetHighestPriorityCC(CurrentCCSubState); }

    // во сколько раз уменьшается время повторного контроля
    UPROPERTY(EditAnywhere, meta = (ClampMin = "1.0", UIMin = "1.0"))
    float CCLockTimeDivider;

    // время, через которое сбрасывается станлок, в секундах
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float CCLockTime;
    
private:
    struct FCCTimers
    {
        FTimerHandle CCTimer;
        FTimerHandle StunLockTimer;
    };

    void ResetCCLock(int index);
    float GetCCLockTimeModifier(ECCState Mode) const;

    ACharacterCreature* Character;
    // массив пар таймер-счетчик станлока, где индекс соответствует енуму ECCState
    TPair<FCCTimers,int> CCLocks[ECCState::NumOfModes];
    unsigned int CurrentCCState; 
    unsigned int CurrentCCSubState;
};


// used to start camera shakes for player if got CC'ed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCCStateChanged, unsigned int, NewState, unsigned int, NewSubState);


class UCharacterMeshComponent;
UCLASS()
class MADFIRE_API ACharacterCreature : public ACharacterBase
{
    GENERATED_BODY()

    friend struct FCCStateManager;

public:
    // Sets default values for this character's properties
    ACharacterCreature(const FObjectInitializer &ObjectInitializer);

    float TakeDamage(float DamageAmount, const struct FDamageEvent &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;
    virtual void Update(const FCharacterLevelInfo &levelInfo);
    UFUNCTION(BlueprintCallable, Category = "Camera")
    virtual void SetAbilityCameraMode(TSubclassOf<UCameraMode> CameraMode) {}
    UFUNCTION(BlueprintCallable, Category = "Camera")
    virtual void ClearAbilityCameraMode() {}
    //virtual void FireLoadedProjectile(); //TODO: Move to combat comp

    void StartAnimation(UAnimMontage *AnimMontage,
                        const FOnMontageBlendingOutStarted &EndCallBack, float InPlayRate,
                        FName StartSectionName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Character Animation")
    void InterruptAnimation(ECCState Source, int SubMode = 0);

    UHumanoidMovementComponent*         GetMovementComponent() const override;
    UCharacterMeshComponent*            GetMeshComponent() const;    
    virtual UInventoryComponent*        GetInventoryComponent() const { return InventoryComponent; }    
    virtual UEquipmentComponent*        GetEquipmentComponent() const { return EquipmentComponent; }    
    virtual UCombatComponent*           GetCombatComponent() const { return CombatComponent; }    
    virtual UAbilityManagerComponent*   GetAbilityManager() const { return AbilityManager; }
    //NOTE: Короче, тут сецификаторы функций не нужны, а виртуальные они, если у наследников изменятся

    UFUNCTION(BlueprintCallable, Category = "Character")
    void DoLevelUp(const FCharacterLevelInfo &levelInfo);

    UFUNCTION(BlueprintCallable, Category = "Character")
    bool PickupItem(AItemActor *ItemActor);

    // TODO уточнить при балансировке
    FORCEINLINE float CalculateMaxWeight() const
    { return MaxWeightBase + MaxWeightModBase * UAttributeSetHelper::GetModifier(GetAttributeComponent()->Attributes, EAttributes::Str); }
    // TODO уточнить при балансировке
    float CalculateKnockDownTime() const;
    void UpdateCCLOBAnims();
    
    // Deprecated
    //UPROPERTY(EditAnywhere, BlueprintReadWrite,
    //          meta = (Bitmask, BitmaskEnum = "ECCState", Category = "Character Stats"))
    //int CurrentCCState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    FCCStateManager CCStateManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats")
    float MaxWeight;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats")
    float KnockDownTime;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float KnockDownBaseTime;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float KnockDownModBase;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float KnockDownWeightModBase;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float MaxWeightBase;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float MaxWeightModBase;


    UCharacterAnimInstance* AnimInstance;

private:
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UInventoryComponent>     InventoryComponent;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UEquipmentComponent>     EquipmentComponent;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere,Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UCombatComponent>        CombatComponent;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere,Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UAbilityManagerComponent> AbilityManager;

protected:
    void PostInitializeComponents() override;
    void BeginPlay() override;

    // clang-format off
    UFUNCTION(BlueprintCallable, Category = "Character Animation")
    void CCStunEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION(BlueprintCallable, Category = "Character Animation")
    void CCLOBEnded(UAnimMontage *animMontage, bool bInterrupted); 
    UFUNCTION(BlueprintCallable, Category = "Character Animation")
    void CCKnockDownEnded(UAnimMontage *animMontage, bool bInterrupted);

    TMap<ECCState, void (ACharacterCreature::*)(UAnimMontage *, bool)> CCInterruptDelegates;
    TMap<ECCState, UAnimMontage *> CCAnims;
    // loss of balance sub mode anims
    //TMap<ELossOfBalanceCCSubState, UAnimMontage *> CCLOBAnims;
    FCCStateChanged OnCCStateChanged;
};
