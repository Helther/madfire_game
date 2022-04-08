#pragma once

#include "../../Items/ItemWeapon.h"
#include "../../CombatSystem/CombatStance.h"
#include "../../AbilitySystem/AbstractAbility.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "CombatComponent.generated.h"

// use it to bind callbacks from related component to link them
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatStateChanged, int, OldState, int, NewState);


enum ECombatState
{
    Idle = 0,
    WeaponSwitch,
    Attack,
    Block,
    Parry,
    Ability
};

enum ECombatStateAnimInfo
{// Set in anim callBacks (for the most part)
    CollisionOn        = 0x0001, // currentWeapon collision
    ComboSaved         = 0x0002,
    PauseComboSaved    = 0x0004, // is set when saving combo after ComboCanContinue is set and ComboSaved not
    MovementBlocked    = 0x0008, // is set at the start of action and reset close to finish
    CanBeParried       = 0x0010,
    ComboCanContinue   = 0x0020, // is set when saved previously combo should play out
    BlockOrParryActive = 0x0040,
    WeaponGrabbed      = 0x0080, // is set at the moment of weapon switch when character reaches his weapon
    AnimEnded          = 0x0100,
    UserInterrupt      = 0x0200, // is set before restarting current montage at different section, to not trigger normal interrupt
    RotationBlocked    = 0x0400, // is set in sync with MovementBlocked, but for combo anims, resets just before ComboCanContinue is set, to allow character to realign with camera
    ChargeAllowed      = 0x0800, // is set when starting combo attack and has chargable atacks, unset when released attack
    CanCounterChamber  = 0x1000, // is set during chamber to indicate that character can attack again, unset after exit chamber mode
    CheckedForChamber  = 0x2000  // is set during chamber to indicate that there is no need to check again with same character
};

// directions of special attack moves
enum ESpecialDirection
{
    SFORWARD = 1,
    SBACK = 2,
    SLEFT = 4,
    SRIGHT = 8
};


struct CombatStateInfo : public StateInfoBase
{
    float ChargeTime = 0.f;
    int ChargeStage = -1;
    // pointer to relevant attack info from current stance
    FAttackInfo *AttackInfo = nullptr;

    void ChangeAnimInfo(uint16 mask, bool set) { set ? (AnimInfo |= mask) : (AnimInfo &= ~mask); }
    bool CheckAnimInfo(uint16 mask) const { return (AnimInfo & mask) == mask; }
    void ResetAnimInfo() { AnimInfo = 0u; }
    
private:
    uint16 AnimInfo = 0u;
};

class UEquipmentComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MADFIRE_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatComponent(const class FObjectInitializer &ObjectInitilizer);

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

    // for checking if allowed to do other combat related actions
    bool IsBusy() const { return CombatFSM.currentState()->Id != ECombatState::Idle; }
    void WeaponsSetup(bool draw);
    // called in attack collision anim notify
    void SetCurrentWeaponCollision(bool main, bool enable);

    // exclusively used to set new states(do not set directly from fsm!)
    bool UpdateCombatFSM();
    void ResetCurrentCombo() { CurrentCombo = 0; }
    // checks if can combo further and starts the animation
    void StartNextComboAttack(bool IsPauseAttack);
    UFUNCTION(BlueprintCallable)
    UItemWeapon* GetCurrentWeapon(bool MainHand) const { return MainHand ? CurrentWeaponMainH : CurrentWeaponOffH; }

//================================= Stance Logic ================================//
    void OnStanceChanged();
    UCombatStance* GetCurrentCombatStance() const;
protected:
    void ResolveNewStance();
    void UpdateStanceAnims(const UCombatStance* DefaultStance);
    bool DoesStanceFit(const UCombatStance* Stance) const;

    TArray<UCombatStance*> StanceStack;
    // set it in order to have fallback stance during stance resolve
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
    UCombatStance* BackupStance;


public:
    FORCEINLINE bool IsBlockButtonPressed() const { return BlockPressed; }
    void ResetRotationRate();

    void UseAimComponent(class UAbilityAimComponent *);

    UFUNCTION()
    void OnMovementModeChanged(int oldMode, int newMode);

    UFUNCTION()
    void OnEquipmentUpdate(UEquipmentComponent* Component, EItemSlot Slot);

    UFUNCTION()
    void OnEquipmentSwap(UEquipmentComponent* Component, EItemSlot SlotL, EItemSlot SlotR);

    // current active directions for special moves
    UPROPERTY(BlueprintReadOnly)
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackSpeed;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UnarmedBaseDamage;
    // максимальный угол между направлениями бойцов
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxFacingAngle;
    ComboFSM<CombatStateInfo> CombatFSM;
    FCombatStateChanged OnCombatStateChanged;
    UItemWeapon *CurrentWeaponMainH = nullptr;
    UItemWeapon *CurrentWeaponOffH = nullptr;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability System")
    UAbstractAbility *CurrentAbility;


    float CalculateWeaponDamage(const UItemWeapon* weapon, float attackInfoDamageMod, float strMod);
    // для расчета возможности применения блока или паррирования
    static float GetAngleBetweenAttackerAndDefender(const AActor *Attacker, const  AActor *Defender);
    // для расчета разницы направлений
    static float GetAngleBetweenActors(const AActor *First, const AActor *Second);

    FORCEINLINE static float CalculateHitForce(float weaponWeight, float strMod, float mod) 
        { return FMath::Max(weaponWeight + strMod, 1.f) * mod; } // TODO улучшить при балансировке

    FORCEINLINE static float CalculateBalance(int32 skillVal, int32 strMod, float shieldWeight) 
        { return FMath::Max(skillVal + strMod + shieldWeight, 1.f); } // TODO улучшить при балансировке

    static FName GetSlotFromWeaponType(EWeaponType type, bool draw, bool mainHand);

protected:
    enum class EChamberRes
    {
        ContinueAttack, // this character force has overwhelmed the enemy and he went on with attack
        ChamberState    // this character bounced back from collision with enemy weapon
    };

    void BeginPlay() override;
    void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction *ThisTickFunction) override;

    void InitCombatFSM();
    // action states are attack and parry, so far
    void ResetActionState(int StateId);
    void CalculateMaxCombatSpeeds();
    void CalculateMaxBlockingSpeed(float SpeedModifier);
    bool CanBeInterruptedWithBlock(int CurrentState) const;
    void WeaponSetup(UItemWeapon* weapon, AItemActor* actor, bool draw, bool mainHand);
    bool HasAlreadyHitTarget(AActor *Target, UItemWeapon* Weapon) const;
    UFUNCTION()
    void OnWeaponOverlap(UPrimitiveComponent* comp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
    bool IsInActionState() const;
    bool IsChamberValid(const AActor* HitActor, const UItemWeapon* ActiveWeapon) const;
    EChamberRes ResolveChamber(AActor* HitActor, const UItemWeapon* OwnerActiveWeapon);
    void EnterChamberState(ACharacterCreature* ChamberedCharacter);
    void SetupDefaultWeapon(bool IsMainHand, bool Equip);

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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
    UItemWeapon *DefaultWeaponMainH = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
    UItemWeapon *DefaultWeaponOffH = nullptr;
    // TODO temp, set before drawing weapon with user selected ability in mind via resolver
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
    UCombatStance *CurrentCombatStance = nullptr;
    // works the same for ai controller, but without actually pressing buttons
    bool WeaponSwitchPressed = false;
    bool AttackPressed = false;
    bool BlockPressed = false;
    // set of characters hit and with what weapon
    TSet<TPair<AActor*, UItemWeapon*>> CharactersHit;
    class ACharacterCreature *Character;
    AItemActor *CurrentMainWeaponActor;
    AItemActor *CurrentOffWeaponActor;
    bool IsWeaponDrawn;

    //================================= Ability System ================================//
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
    TSet<UAbstractAbility*> AbilityList;

    bool AbilityPressed = false;
    FTimerHandle GlobalCooldownTimer;

private:
    bool IsDirty = false;
    int CurrentCombo = 0;
    // TODO use this to check if equipped melee weapon or not
    TSet<EWeaponType> RangedWeaponTypes;
};
