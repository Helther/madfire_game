#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "CCComponent.generated.h"

// used to start camera shakes for player if got CC'ed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCCStateChanged, unsigned int, NewState, unsigned int, NewSubState);

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

UENUM(BlueprintType)
// Combat oriented Character Control sub CC modes (value represents priority order(the more the better))
enum ELossOfBalanceCCSubState
{
    None = 0,
    HitWall,
    BlockedAttack,
    WasParried,
    Chamber // ударил об оружие противника
};

/**
 * Компонент отвечающий за потерю контроля контроллером над персонажем.
 * Подразумавется что владелец будет наследован от персонажа.
 * Здесь реализованы установка и сброс в множество режимов одновременно с учетом приоритета важности (более важные имеют большие значения в енуме режимов).
 * Так же реализована механика стан-лока с убывающим временем контроля при множественных установках одного и того же режима.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UCCComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCCComponent(const class FObjectInitializer &ObjectInitializer);
    // Called when the game starts
    virtual void BeginPlay() override;
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    static int GetHighestPriorityCC(int bitmask);

    FORCEINLINE bool HasCC() const { return CurrentCCState != ECCState::NoCC; }
    FORCEINLINE bool IsInCCMode(ECCState Mode) const { return CurrentCCState & (1u << Mode); }
    FORCEINLINE bool IsInCCSubMode(unsigned int SubMode) const { return CurrentCCSubState & (1u << SubMode); }
    FORCEINLINE bool CheckCCMode(ECCState Mode, unsigned int SubMode) const { return (CurrentCCState & (1u << Mode)) && (CurrentCCSubState & (1u << SubMode)); }
    // sets flag, starts cc timer and stunLock timer, time in seconds (time null means time is equal to animation duration)
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

    FCCStateChanged OnCCStateChanged;
    TMap<ECCState, UAnimMontage *> CCAnims;
    // loss of balance sub mode anims
    TMap<ELossOfBalanceCCSubState, UAnimMontage *> CCLOBAnims;

private:
    struct FCCTimers
    {
        FTimerHandle CCTimer;
        FTimerHandle StunLockTimer;
    };
    UFUNCTION()
    void ResetCCLock(int index);
    float GetCCLockTimeModifier(ECCState Mode) const;

    // массив пар таймер-счетчик станлока, где индекс соответствует енуму ECCState
    TPair<FCCTimers,int> CCLocks[ECCState::NumOfModes];
    unsigned int CurrentCCState; 
    unsigned int CurrentCCSubState;
};

