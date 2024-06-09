#pragma once

#include "../Utils/FSM.h"
#include "../Rules/GameRules.h"
#include "CoreMinimal.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/BlendSpace.h" 
#include "Templates/SharedPointer.h"
#include "CombatStance.generated.h"



UENUM(BlueprintType)
enum class ESpecialType : uint8 
{
    None = 0,
    Air,
    Forward,
    Back,
    Left,
    Right,
    Sprint,
    Crouch
};
ENUM_CLASS_FLAGS(ESpecialType)

UENUM(BlueprintType)
enum class EWeaponCombination : uint8 
{
    TwoWeapon = 0,    // Need weapon in each hand to use (exludes weapon+shield combination)
    TwoHand,
    OneHand,
    Shield,           // Need One handed weapon and shield
    Ranged
};
ENUM_CLASS_FLAGS(EWeaponCombination)

UENUM(BlueprintType)
// Combat oriented Character Control sub CC modes (value represents priority order(the more the better))
enum class ELossOfBalanceCCSubState : uint8
{
    None = 0,
    HitWall,
    BlockedAttack,
    WasParried,
    Chamber, // ударил об оружие противника
    MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECombatMovementType : uint8 
{
    Block = 0,
    Parry, 
    Draw,
    Sheath,
    Reload,  
    MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECombatMotion : uint8 
{
    Stand = 0,
    Crouch, 
    MAX UMETA(Hidden)
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCustomAttackEvent, AActor *, AttackActor, AActor *, DefendActor);

USTRUCT(BlueprintType)
struct FAttackInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    class UAnimMontage *AttackMontage;
    UPROPERTY(EditAnywhere)
    FName AnimSectionName;
    UPROPERTY(EditAnywhere)
    float DamageMod;
    UPROPERTY(EditAnywhere)
    float PowerMod;
    UPROPERTY(EditAnywhere)
    float StaminaCost;
    UPROPERTY(EditAnywhere)
    bool IsNotBlockable;
    UPROPERTY(EditAnywhere)
    bool IsNotParriable;
    UPROPERTY(EditAnywhere)
    bool IsNotInterruptable;
    UPROPERTY(EditAnywhere)
    ESpecialType Special;

    // Delegate for special on hit action
    UPROPERTY(BlueprintAssignable)
    FCustomAttackEvent CustomOnHitEvent;
};

USTRUCT(BlueprintType)
struct FChargeAttackInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChargeTimeNeeded = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAttackInfo Info;
};

USTRUCT(BlueprintType)
struct FComboInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FChargeAttackInfo> ChargeAttackStages;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAttackInfo NormalAttack;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAttackInfo PauseAttack;
};

struct CombatStateInfo : public StateInfoBase
{
    float ChargeTime = 0.f;
    int ChargeStage = -1;
    // pointer to relevant attack info from current stance
    const FAttackInfo *AttackInfo = nullptr;

    FORCEINLINE void ChangeAnimInfo(uint16 mask, bool set) { set ? (AnimInfo |= mask) : (AnimInfo &= ~mask); }
    FORCEINLINE bool CheckAnimInfo(uint16 mask) const { return (AnimInfo & mask) == mask; }
    FORCEINLINE void ResetAnimInfo() { AnimInfo = 0u; }
    
private:
    uint16 AnimInfo = 0u;
};

enum ECombatState
{
    Idle = 0,
    WeaponSwitch,
    Attack,
    Block,
    Parry,
    Ability,
    Reload
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
    CheckedForChamber  = 0x2000, // is set during chamber to indicate that there is no need to check again with same character
    AmmoGrabbed        = 0x4000, // is set during reload, when reached the ammo
    AmmoAttach         = 0x8000  // set during reload, when actor brought ammo to the weapon
};

UENUM(BlueprintType)
enum class EArmsSlot: uint8
{
    MainHand = 0,
    OffHand,
    Shield,
    RightFoot,
    LeftFoot
};
ENUM_CLASS_FLAGS(EArmsSlot)

USTRUCT(BlueprintType)
struct FWeaponInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class AItemActor> ItemActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool Shield;
};

class UCombatComponent;
class ACharacterCreature;

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew)
class MADFIRE_API UCombatStance : public UPrimaryDataAsset
{
    GENERATED_BODY()

    friend class UCombatComponent;
public:
    virtual const FAttackInfo &GetCurrentAttackInfo(ESpecialType type = ESpecialType::None) const;

    virtual bool IsLoaded() const;

    virtual bool HasChargeAttack() const;

    //TODO: Better name needed
    virtual bool CanDoShit(ESpecialType type = ESpecialType::None) const;

    bool CanDoShit(ECombatMovementType type) const;

    bool CanDoShit(ECombatMotion type) const;

    virtual void StanceSet(ACharacterCreature* NewOwner);

    virtual void StanceUnSet();

    // check that weapons setup correctly
    virtual bool IsStanceValid(const ACharacterCreature *NewOwner) const;

    // custom blueprint specific check
    UFUNCTION(BlueprintPure)
    virtual bool IsMelee() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Stance Interface")
    bool CheckRequirement(ACharacterCreature* character) const;
    bool CheckRequirement_Implementation(ACharacterCreature* character) const { return true; }

    /// Must implement in BP
    // calls WeaponSetup for all weapons, attaches the weapons
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    void ArmsSetup(bool Draw);

    // add check that vars are not setup, or else ...
    //UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    //void SetWeaponVars(ACharacterCreature* NewOwner);

    //UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    //void ResetWeaponVars();

    // returns correct variable for a given slot
    //UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    //UItemArms* GetArmsFromSlot(EArmsSlot Slot) const;

    //UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    //AItemActor* GetArmsActorFromSlot(EArmsSlot Slot) const;
    
    UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category = "Stance Interface")
    void AllocateWeapons(ACharacterCreature* NewOwner, TMap<EArmsSlot, FWeaponInfo> &Result) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsUsingDefaultWeapons = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeaponCombination StanceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsBlockAllowed = true;
    // a set of weapon types in which character equipped weapons should all be present
    // in order to activate this stance
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSet<EWeaponType> AllowedWeaponTypes;

    ////////////////////////// combat mode speeds//////////////////////////
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Speeds")
    float MaxCombatRunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Speeds")
    float MaxCombatCrouchedSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Speeds")
    float MaxCombatSprintSpeed;

    // Max move speed during blocking or attacking if allowed (charging drawing)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Speeds")
    float MaxBlockOrActionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAimOffsetBlendSpace *AimOffset;

    UPROPERTY(EditDefaultsOnly, meta=(ArraySizeEnum=ECombatMovementType))
    UAnimMontage *CombatMoves[ECombatMovementType::MAX];
    
    UPROPERTY(EditDefaultsOnly, meta=(ArraySizeEnum=ECombatMovementType))
    UBlendSpace *CombatMotions[ECombatMotion::MAX];

    UPROPERTY(EditDefaultsOnly, meta=(ArraySizeEnum=ELossOfBalanceCCSubState))
    UAnimMontage *CCLOBAnims[ELossOfBalanceCCSubState::MAX];

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsParryAllowed = true;
    UPROPERTY(EditAnywhere)
    float ParryStaminaCost = 15;


protected:
    /// must implement in BP
    // sets buffs/debuffs and spawn default weapon for unarmed stance
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    void OnStanceActivated();
    // resets buffs/debuffs and destroys default weapons (check if spawned it in the first place)
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    void OnStanceDeActivated();

    // used for req checks, set when gets stance resolved
    UPROPERTY(BlueprintReadOnly)
    ACharacterCreature* Owner = nullptr;
};


UCLASS(BlueprintType, Blueprintable)
class UMeleeCombatStance : public UCombatStance
{
    GENERATED_BODY()

public:
    UMeleeCombatStance();

    bool IsStanceValid(const ACharacterCreature* NewOwner) const override;

    bool IsMelee() const override;

    // use size as max combo count
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FComboInfo> ComboAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAttackInfo> SpecialAttacks;

    const FAttackInfo &GetCurrentAttackInfo(ESpecialType type) const override;

    bool CanDoShit(ESpecialType type) const override;

    bool HasChargeAttack() const override;
};

// load ammo handling, make a copy of ammo actor and store it in a stance, substact from equipment ammo
// if fired the ammo or resetstance add ammo item to inventory and destroy the actor
class AItemActorProjectile;
class UItemWeaponRanged;
UCLASS(Abstract, BlueprintType, Blueprintable)
class URangeCombatStance : public UCombatStance
{
    GENERATED_BODY()

public:
    URangeCombatStance();

    bool IsStanceValid(const ACharacterCreature* NewOwner) const override;

    bool IsMelee() const override { return false; }

    void StanceSet(ACharacterCreature* NewOwner) override;

    void StanceUnSet() override;

    void FireLoadedAmmo();
    // called when character reached ammo during reload
    void SpawnAttachAmmoActor();

    void AttachAmmoActorToWeapon();

    const FAttackInfo &GetCurrentAttackInfo(ESpecialType type) const override;

    bool IsLoaded() const override {return IsWeaponLoaded;}

    bool CanDoShit(ESpecialType type) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAttackInfo AttackInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAttributes AnimRateMod;

    UPROPERTY()
    AItemActorProjectile* LoadedAmmo = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReloadStaminaCost = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 LoadAmmoCount = 1;

    UPROPERTY(BlueprintReadOnly)
    bool IsWeaponLoaded = false;

protected:
    // void UpdateOwnerAnims(const UCombatStance* DefaultStance) override;
    virtual float CalculateAmmoInitialSpeed(UItemWeaponRanged* Weapon) const;
    // called when switched stance
    void CleanUpAmmoActor();
};


UCLASS(BlueprintType, Blueprintable)
class UBowCombatStance : public URangeCombatStance
{
    GENERATED_BODY()

public:
    UBowCombatStance();

    UFUNCTION(BlueprintCallable)
    float GetDrawRatio() const;

    float GetMaxDrawTime() const;

    float GetLastDrawRatio() const;

    // set same as corresponding segment in attack montage for bow drawing
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAnimSequence* BowDrawingAnim;

    // time in seconds between stamina consume tick during draw
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChargeStaminaConsumeDelta;

    float LastChargeTimeStaminaConsume;

    float LastChargeTime;

private:    
    float CalculateAmmoInitialSpeed(UItemWeaponRanged* Weapon) const override;
};


UCLASS(BlueprintType, Blueprintable)
class UCrossbowCombatStance : public URangeCombatStance
{
    GENERATED_BODY()

public:
    UCrossbowCombatStance();
};
