#pragma once

#include "../../Items/ItemEquipment.h"
#include "../../CombatSystem/CombatStance.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "CombatComponent.generated.h"

// use it to bind callbacks from related component to link them
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatStateChanged, int, OldState, int, NewState);


// directions of special attack moves
enum ESpecialDirection
{
    SFORWARD = 1,
    SBACK = 2,
    SLEFT = 4,
    SRIGHT = 8
};

USTRUCT(BlueprintType)
struct FStanceResolveResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool NoValidStanceFound = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool NewStanceWasSet = false;
};


class UEquipmentComponent;
class ACharacterCreature;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

    friend class UCombatStance;
    friend class UMeleeCombatStance;
    friend class URangeCombatStance;
    friend class UBowCombatStance;
    friend class UCrossbowCombatStance;

public:
    UCombatComponent(const class FObjectInitializer &ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void Update();

    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void SetUpdate(bool Update) { IsDirty = Update; }

    UFUNCTION(BlueprintCallable, Category = "Component Update")
    bool IsUpdateNeeded() { return IsDirty; }

    UFUNCTION(BlueprintCallable)
    bool WeaponDrawn() const { return IsWeaponDrawn; }

    // returns nullptr if not
    UFUNCTION(BlueprintCallable)
    UBowCombatStance* IsDrawingBow() const;

    // Infers what stance should be from current equipment (if stance is specified in the argument, then then check it before/instead looking in StanceStack)
    // WeaponChanged is true then equipment was updated
    UFUNCTION(BlueprintCallable)
    FStanceResolveResult ResolveNewStance(UCombatStance* NewStance = nullptr, bool WeaponChanged = false);
    
    UFUNCTION(BlueprintCallable)
    FORCEINLINE bool HasCombatStance() const { return CurrentCombatStance != nullptr; }
    
    UFUNCTION(BlueprintCallable)
    FORCEINLINE UCombatStance *GetCurrentCombatStance() const { return CurrentCombatStance; }
    
    UFUNCTION(BlueprintCallable)
    void ToggleHandMode();

    void ResetRotationRate();

    UFUNCTION()
    void OnMovementModeChanged(int oldMode, int newMode);

    UFUNCTION()
    void OnEquipmentUpdate(UEquipmentComponent* Component, EItemSlot Slot);

    UFUNCTION()
    void OnEquipmentSwap(UEquipmentComponent* Component, EItemSlot SlotL, EItemSlot SlotR);
    bool IsWeaponSwapAllowed(EItemSlot lhs, EItemSlot rhs) const;

    FORCEINLINE bool IsBlockButtonPressed() const { return BlockPressed; }
    FORCEINLINE bool IsWeaponSwitchPressed() const { return WeaponSwitchPressed; }
    FORCEINLINE bool IsAttackPressed() const { return AttackPressed; }
    FORCEINLINE bool IsBlockPressed() const { return BlockPressed; }

    // Anim CallBacks
    UFUNCTION()
    void OnWeaponSwitchEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION()
    void OnBlockEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION()
    void OnParryEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION()
    void OnAttackEnded(UAnimMontage *animMontage, bool bInterrupted);  

    void OnAttack(bool Started);
    void OnBlock(bool Started);
    void OnDrawWeapon();

    // setups collision and attaches weapon to owner if has not been attached already by equipment
    UFUNCTION(BlueprintCallable)
    void WeaponSetup(UItemArms* Weapon, AItemActor* Actor, bool Draw, FName Slot, bool MainHand);
    
    // returns nullptr if no such weapon equipped
    UFUNCTION(BlueprintCallable, Category = "Ranged Combat")
    UItemWeaponRanged* GetEquippedRangedWeapon() const;

    // Add Aim Spread due to movement, can be added at when CameraMoveSpreadAddTimeOut seconds past
    UFUNCTION(BlueprintCallable, Category = "Ranged Combat")
    void AddRangedWeaponSpread();
    
    // called in attack collision anim notify
    //FORCEINLINE  auto& GetCombatFSM() const { return CurrentCombatStance->CombatFSM; }
    FORCEINLINE const auto& GetCombatFSM() const { return CombatFSM; }
    FORCEINLINE auto& GetCombatFSM() { return CombatFSM; }

    // for checking if allowed to do other combat related actions
    UFUNCTION(BlueprintPure)
    FORCEINLINE bool IsBusy() const { return HasCombatStance() && !GetCombatFSM().isInState(ECombatState::Idle); }
    // exclusively used to set new states(do not set directly from fsm!)

    bool UpdateCombatFSM();
    // checks if can combo further and starts the animation

    void StartNextComboAttack(bool IsPauseAttack);

    float CalculateWeaponDamage(const UItemArms* weapon, float attackInfoDamageMod, float strMod);

    UFUNCTION(BlueprintCallable)
    static void SetWeaponCollision(AItemActor* WeaponActor, bool Enable);
    UFUNCTION(BlueprintCallable)
    static FName GetSlotFromWeaponType(EWeaponType type, bool draw, bool mainHand);
    // для расчета возможности применения блока или паррирования
    static float GetAngleBetweenAttackerAndDefender(const AActor *Attacker, const AActor *Defender);
    // для расчета разницы направлений
    static float GetAngleBetweenActors(const AActor *First, const AActor *Second);

    FORCEINLINE static float CalculateHitForce(float weaponWeight, float strMod, float mod) 
        { return FMath::Max(weaponWeight + strMod, 1.f) * mod; } // TODO улучшить при балансировке

    FORCEINLINE static float CalculateBalance(int32 skillVal, int32 strMod, float shieldWeight) 
        { return FMath::Max(skillVal + strMod + shieldWeight, 1.f); } // TODO улучшить при балансировке


    
    // in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged Combat", Meta = (UIMin = "0.0", ClampMin = "0.0"))
    double CameraMoveSpreadAddTimeOut;

    // current active directions for special moves
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    uint8 SpecialDirections = 0u;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (UIMin = "0.0", ClampMin = "0.0"))
    float BlockStaminaCost;

    // max ratio of hit power to character balance to not get kd
    // should be higher than 1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (UIMin = "1.0", ClampMin = "1.0"))
    float KnockdownThreshhold;

    // max ratio between characters attack powers during chamber 
    // (if ration is lower than max, then both charactera enter chamber mode)
    // should be higher than 1 and lower than KnockdownThreshhold
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (UIMin = "1.0", ClampMin = "1.0"))
    float ChamberThreshhold;

    // значение относительно нормы (коэффициент)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float AttackSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool TwoHandMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UnarmedBaseDamage;

    // максимальный угол между направлениями бойцов
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxFacingAngle;

    // damage modifier for shield from it's weight 
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShieldWeightDamageMod;

    UPROPERTY(BlueprintAssignable)
    FCombatStateChanged OnCombatStateChanged;

    void CalculateMaxCombatSpeeds();

protected:
    struct PendingStanceInfo
    {
        // Use it to swap stances with weapons drawn

        // check if state is valid and return true if new stance is pending switch
        FORCEINLINE bool IsSet() const { check(!((NewStance != nullptr) ^ (DefaultStance != nullptr))); return NewStance && DefaultStance; }
        FORCEINLINE void Reset() { NewStance = nullptr; DefaultStance = nullptr; }

        UCombatStance *NewStance = nullptr;
        const UCombatStance *DefaultStance = nullptr;
    };

    enum class EChamberRes
    {
        ContinueAttack, // this character force has overwhelmed the enemy and he went on with attack
        ChamberState    // this character bounced back from collision with enemy weapon
    };

    void BeginPlay() override;
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    // action states are attack and parry, so far
    void ResetActionState(int StateId);    
    void CalculateMaxBlockingSpeed(float SpeedModifier);
    bool CanBeInterruptedWithBlock(int CurrentState) const;
    bool HasAlreadyHitTarget(AActor *Target, UItemArms* Weapon) const;
    UFUNCTION()
    void OnWeaponOverlap(UPrimitiveComponent* comp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
    UFUNCTION()
    void OnRangedWeaponOverlap(UPrimitiveComponent* comp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
    bool IsInActionState() const;
    bool IsChamberValid(const ACharacterCreature* HitCharacter, const UItemArms* HitActorActiveWeapon, const UItemArms* OwnerActiveWeapon) const;

    // false - character force has overwhelmed the enemy and he went on with attack
    // true -  character bounced back from collision with enemy weapon
    bool ResolveChamber(ACharacterCreature* HitCharacter, const UItemArms* HitActorActiveWeapon, const UItemArms* OwnerActiveWeapon);
    void EnterChamberState(ACharacterCreature* ChamberedCharacter);

    void SetNewStance(UCombatStance* OldStance, UCombatStance* NewStance, const UCombatStance* DefaultStance);
    void StanceChanged(UCombatStance* OldStance, UCombatStance* NewStance, const UCombatStance* DefaultStance, bool WeaponChanged);
    bool DoesStanceFit(UCombatStance* Stance, bool CheckHandMode = false) const;
    UCombatStance* CheckStances(const TArray<UCombatStance*>& Stances, bool CheckHandMode = false);
    // looks in current stance for a weapon that has the given collision
    UItemArms* FindHitMeleeWeapon(const UPrimitiveComponent* HitComp) const;
    // checks if given collision hit this characters weapon
    static bool HitOwnMeleeWeapons(const UPrimitiveComponent* HitComp, const AActor* HitActor);
    static bool CheckActorCollisionActive(const AItemActor* WeaponActor);


    // looks in current stance for weapons have enabled collision
    TArray<AItemActor*> FindActiveMeleeWeapons() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UCombatStance *CurrentCombatStance;

    PendingStanceInfo PendingStance;

    TArray<UCombatStance*> StanceStack;

    // character orientation to movement speed while weapon is drawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CombatModeRotationRate;

    // character orientation to movement speed while playing root animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActionRotationRate;

    // time of holding attack needed for charge anim to start playing, should be higher than 0.5s or will trigger charge with speed clicking
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChargeStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlockStaminaRegenRateMod;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DrawTimeModFromTension;

    // set of characters hit and with what weapon
    TSet<TPair<AActor*, UItemArms*>> CharactersHit;

    // works the same for ai controller, but without actually pressing buttons
    bool WeaponSwitchPressed = false;

    bool AttackPressed = false;

    bool BlockPressed = false;

    ACharacterCreature *Owner = nullptr;

    bool IsWeaponDrawn;


private:
    bool IsDirty = false;


public: //New Revision
    ESpecialType GetSpecialType(bool &isDir) const;

    FSMv2<CombatStateInfo> CombatFSM;
    
    void InitFSM();

    bool InState(ECombatState State);

    //FAttackInfo GetCurrentAttackInfo();
    UFUNCTION()
    bool IdleToBlock() const;
    UFUNCTION()
    bool IdleToAttack() const;
    UFUNCTION()
    bool IdleToWeaponSwitch() const;
    UFUNCTION()
    bool IdleToReload() const;
    UFUNCTION()
    bool WeaponSwitchToIdle() const;
    UFUNCTION()
    bool AttackToBlock() const;
    UFUNCTION()
    bool AttackToIdle() const;
    UFUNCTION()
    bool BlockToParry() const;
    UFUNCTION()
    bool BlockToIdle() const;
    UFUNCTION()
    bool ParryToBlock() const;
    UFUNCTION()
    bool ParryToIdle() const;
    UFUNCTION()
    bool ReloadToAttack() const;
    UFUNCTION()
    bool ReloadToIdle() const;
    UFUNCTION()
    void OnIdleEnter();
    UFUNCTION()
    void OnIdleLeave();
    UFUNCTION()
    void OnAttackEnter();
    UFUNCTION()
    void OnAttackLeave();
    UFUNCTION()
    void OnSwitchEnter();
    UFUNCTION()
    void OnSwitchLeave();
    UFUNCTION()
    void OnBlockEnter();
    UFUNCTION()
    void OnBlockLeave();
    UFUNCTION()
    void OnParryEnter();
    UFUNCTION()
    void OnParryLeave();
    UFUNCTION()
    void OnReloadEnter();
    UFUNCTION()
    void OnReloadLeave();

    void OnReloadEnded(UAnimMontage* AnimMontage, bool bInterrupted);

    void UpdateOwnerAnims(const UCombatStance* DefaultStance);

    UFUNCTION(BlueprintPure)
    bool IsRangedDraw(float &DrawRatio) const;

    float GetMaxDrawTime(UBowCombatStance *Stance) const;
    float GetLastDrawRatio() const;
    float CalculateAmmoInitialSpeed(UItemWeaponRanged* Weapon) const;
    UFUNCTION(BlueprintCallable)
    float GetCurrentDrawRatio() const;

    int CurrentCombo = 0; // Must be private or blueprintReadOnly

    void ResetComboCount();

    int GetComboCount() const;

    void IncreaseComboCount();

    UPROPERTY(BlueprintReadOnly)
    bool IsWeaponLoaded = false;

    //FCombatStateChangedNew OnCombatStateChangedN;
    void FireLoadedProjectile();

    float LastChargeTimeStaminaConsume;

    float LastChargeTime;

    TMap<EArmsSlot, FWeaponInfo> WeaponsMap;
        
    AItemActor* GetArmActor(EArmsSlot Slot) const;

    UItemArms* GetArmItem(EArmsSlot Slot) const;

    void ArmsSetup(bool Draw);

    bool IsStanceValid(UCombatStance *Stance) const;
};
