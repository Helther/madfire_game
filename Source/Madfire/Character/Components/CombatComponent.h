#pragma once

#include "../../Items/ItemEquipment.h"
#include "../../CombatSystem/CombatStance.h"
#include "../../AbilitySystem/AbstractAbility.h"
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

    friend class UMeleeCombatStance;

public:
    UCombatComponent(const class FObjectInitializer &ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void Update();
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void SetDirty() { IsDirty = true; }
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void UnSetUpdate() { IsDirty = false; }
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    bool IsUpdateNeeded() { return IsDirty; }

    UFUNCTION(BlueprintCallable)
    bool WeaponDrawn() const { return IsWeaponDrawn; }

    // Anim CallBacks
    UFUNCTION()
    void OnWeaponSwitchEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION()
    void OnBlockEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION()
    void OnParryEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION()
    void OnAttackEnded(UAnimMontage *animMontage, bool bInterrupted);  
    //

    void OnAttack(bool Started);
    void OnBlock(bool Started);
    void OnDrawWeapon();
    void OnUseAbility(bool Started);
    // setups collision and attaches weapon to owner if has not been attached already by equipment
    UFUNCTION(BlueprintCallable)
    void WeaponSetup(UItemArms* Weapon, AItemActor* Actor, bool Draw, FName Slot, bool MainHand);
    // called in attack collision anim notify
    UFUNCTION(BlueprintCallable)
    static void SetWeaponCollision(AItemActor* WeaponActor, bool Enable);
    UFUNCTION(BlueprintCallable)
    static FName GetSlotFromWeaponType(EWeaponType type, bool draw, bool mainHand);
    FORCEINLINE auto& GetCombatFSM() const { return CurrentCombatStance->CombatFSM; }
    // for checking if allowed to do other combat related actions
    UFUNCTION(BlueprintCallable)
    FORCEINLINE bool IsBusy() const { return HasCombatStance() && !GetCombatFSM().isInState(ECombatState::Idle); }
    // exclusively used to set new states(do not set directly from fsm!)
    bool UpdateCombatFSM();
    // checks if can combo further and starts the animation
    void StartNextComboAttack(bool IsPauseAttack);

//================================= Stance Logic ================================//
    // Infers what stance should be from current equipment (if stance is specified in the argument, then then check it before/instead looking in StanceStack)
    // WeaponChanged is true then equipment was updated
    UFUNCTION(BlueprintCallable)
    FStanceResolveResult ResolveNewStance(UCombatStance* NewStance = nullptr, bool WeaponChanged = false);
    FORCEINLINE bool HasCombatStance() const { return CurrentCombatStance != nullptr; }
    FORCEINLINE UCombatStance *GetCurrentCombatStance() const { return CurrentCombatStance; }
    UFUNCTION(BlueprintCallable)
    void ToggleHandMode();
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

    void SetNewStance(UCombatStance* OldStance, UCombatStance* NewStance, const UCombatStance* DefaultStance);
    void StanceChanged(UCombatStance* OldStance, UCombatStance* NewStance, const UCombatStance* DefaultStance, bool WeaponChanged);
    bool DoesStanceFit(UCombatStance* Stance, bool CheckHandMode = false) const;
    UCombatStance* CheckStances(const TArray<UCombatStance*>& Stances, bool CheckHandMode = false);
    // looks in current stance for a weapon that has the given collision
    UItemArms* FindHitMeleeWeapon(const UPrimitiveComponent* HitComp) const;
    // checks if given collision hit this characters weapon
    bool HitOwnMeleeWeapons(const UPrimitiveComponent* HitComp, const AActor* HitActor) const;
    // looks for the first weapon (starting with main-hand) that has active collision
    UItemArms* FindCharacterActiveWeapon(const ACharacterCreature* SomeCharacter) const;
    // looks in current stance for weapons have enabled collision
    TArray<AItemActor*> FindActiveMeleeWeapons() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UCombatStance *CurrentCombatStance;
    PendingStanceInfo PendingStance;
    TArray<UCombatStance*> StanceStack;
//============================================================================

public:
    void ResetRotationRate();

    void UseAimComponent(class UAbilityAimComponent *);

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

    // current active directions for special moves
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    uint8 SpecialDirections = 0u;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlockStaminaCost;
    // max ratio of hit power to character balance to not get kd
    // should be higher than 1
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float KnockdownThreshhold;
    // max ratio between characters attack powers during chamber 
    // (if ration is lower than max, then both charactera enter chamber mode)
    // should be higher than 1 and lower than KnockdownThreshhold
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChamberThreshhold;
    // значение относительно нормы (коэффициент)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float AttackSpeed;
    // TODO add check when switching, that current stance is unarmed or ranged->noop
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool TwoHandMode = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UnarmedBaseDamage;
    // максимальный угол между направлениями бойцов
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxFacingAngle;
    // damage modifier for shield from it's weight 
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShieldWeightDamageMod;

    FCombatStateChanged OnCombatStateChanged;

    float CalculateWeaponDamage(const UItemArms* weapon, float attackInfoDamageMod, float strMod);
    // для расчета возможности применения блока или паррирования
    static float GetAngleBetweenAttackerAndDefender(const AActor *Attacker, const AActor *Defender);
    // для расчета разницы направлений
    static float GetAngleBetweenActors(const AActor *First, const AActor *Second);

    FORCEINLINE static float CalculateHitForce(float weaponWeight, float strMod, float mod) 
        { return FMath::Max(weaponWeight + strMod, 1.f) * mod; } // TODO улучшить при балансировке

    FORCEINLINE static float CalculateBalance(int32 skillVal, int32 strMod, float shieldWeight) 
        { return FMath::Max(skillVal + strMod + shieldWeight, 1.f); } // TODO улучшить при балансировке

    //================================= Ability System ================================//
    UFUNCTION(BlueprintCallable, Category = "Ability System")
    void SetCurrentAbility(UAbstractAbility *Ability);
    UFUNCTION(BlueprintCallable, Category = "Ability System")
    void ActivateAbility();
    UFUNCTION(BlueprintCallable, Category = "Ability System")
    void ReleaseAbility();
    UFUNCTION(BlueprintCallable, Category = "Ability System")
    void AddAbility(UAbstractAbility *Ability);
    UFUNCTION(BlueprintCallable, Category = "Ability System")
    void RemoveAbility(UAbstractAbility *Ability);
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
    TSoftObjectPtr<UAbstractAbility> CurrentAbility;
    FORCEINLINE bool IsAbilityPressed() const { return AbilityPressed; }
    FORCEINLINE bool IsOnGlobalCooldown() const { return GetWorld()->GetTimerManager().IsTimerActive(GlobalCooldownTimer); }
    FORCEINLINE void SetGlobalCooldown() { GetWorld()->GetTimerManager().SetTimer(GlobalCooldownTimer, GlobalCooldownTime, false); }
    UFUNCTION(BlueprintCallable, Category = "Ability System")
    bool IsUsingAbility() const;


protected:
    enum class EChamberRes
    {
        ContinueAttack, // this character force has overwhelmed the enemy and he went on with attack
        ChamberState    // this character bounced back from collision with enemy weapon
    };

    void BeginPlay() override;
    void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction *ThisTickFunction) override;

    // action states are attack and parry, so far
    void ResetActionState(int StateId);
    void CalculateMaxCombatSpeeds();
    void CalculateMaxBlockingSpeed(float SpeedModifier);
    bool CanBeInterruptedWithBlock(int CurrentState) const;
    bool HasAlreadyHitTarget(AActor *Target, UItemArms* Weapon) const;
    UFUNCTION()
    void OnWeaponOverlap(UPrimitiveComponent* comp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
    bool IsInActionState() const;
    bool IsChamberValid(const ACharacterCreature* HitCharacter, const UItemArms* HitActorActiveWeapon, const UItemArms* OwnerActiveWeapon) const;
    EChamberRes ResolveChamber(ACharacterCreature* HitCharacter, const UItemArms* HitActorActiveWeapon, const UItemArms* OwnerActiveWeapon);
    void EnterChamberState(ACharacterCreature* ChamberedCharacter);

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
    // set of characters hit and with what weapon
    TSet<TPair<AActor*, UItemArms*>> CharactersHit;
    // works the same for ai controller, but without actually pressing buttons
    bool WeaponSwitchPressed = false;
    bool AttackPressed = false;
    bool BlockPressed = false;
    ACharacterCreature *Owner = nullptr;
    bool IsWeaponDrawn;

    //================================= Ability System ================================//
    //TODO: Update Delegate For Ability panel
    void CalculateMaxCastingSpeed(float SpeedModifier);
    // значение относительно нормы
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    float CastSpeed;
    //Max move speed during ability cast
    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Ability System")
    float MaxSpeedWhileCasting;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    float GlobalCooldownTime = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability System")
    TArray<UAbstractAbility*> AbilityList;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    TArray<TSoftObjectPtr<UAbstractAbility>> AbilityPanel;

    bool AbilityPressed = false;
    FTimerHandle GlobalCooldownTimer;

private:
    bool IsDirty = false;
};
