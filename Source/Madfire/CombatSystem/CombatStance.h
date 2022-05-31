#pragma once

#include "../Utils/FSM.h"
#include "CoreMinimal.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/BlendSpace.h"
#include "../Rules/GameRules.h"
    
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
    FAttackInfo *AttackInfo = nullptr;

    void ChangeAnimInfo(uint16 mask, bool set) { set ? (AnimInfo |= mask) : (AnimInfo &= ~mask); }
    bool CheckAnimInfo(uint16 mask) const { return (AnimInfo & mask) == mask; }
    void ResetAnimInfo() { AnimInfo = 0u; }
    
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


class UCombatComponent;
class ACharacterCreature;

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew)
class MADFIRE_API UCombatStance : public UObject
{
    GENERATED_BODY()

    friend class UCombatComponent;
public:
    // called when setting to current stance in combat component
    virtual void StanceSwitched(ACharacterCreature* NewOwner, const UCombatStance* DefaultStance) {}
    // called before switching to another stance
    virtual void ResetStance() {}
    // check that weapons setup correctly
    virtual bool IsStanceValid(const ACharacterCreature* NewOwner) const { return true; }
    // custom blueprint specific check
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Stance Interface")
    bool CheckRequirement(ACharacterCreature* character) const;
    bool CheckRequirement_Implementation(ACharacterCreature* character) const { return true; }

    /// Must implement in BP
    // calls WeaponSetup for all weapons, attaches the weapons
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    void ArmsSetup(bool Draw);
    // add check that vars are not setup, or else ...
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    void SetWeaponVars(ACharacterCreature* NewOwner);
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    void ResetWeaponVars();
    // returns correct variable for a given slot
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    UItemArms* GetArmsFromSlot(EArmsSlot Slot) const;
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    AItemActor* GetArmsActorFromSlot(EArmsSlot Slot) const;
    ///

    FAttackInfo* GetAttackInfoForSpecial(ESpecialType type);
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

    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Combat Speeds")
    float MaxCombatSprintSpeed;
    // Max move speed during blocking
    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Combat Speeds")
    float MaxBlockSpeed;
    ////////////////////////// END combat mode speeds//////////////////////////

    FSM<CombatStateInfo> CombatFSM;

    // use size as max combo count
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FComboInfo> ComboAttacks;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAttackInfo> SpecialAttacks;
    // custom equipment specific animations for unique stances
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage *BlockAnim;// TODO modify anim speed according to shield weight after stance resolve
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage *ParryAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UBlendSpace *StandLocomotion;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UBlendSpace *CrouchLocomotion;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAimOffsetBlendSpace *AimOffset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage *DrawAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage *SheathAnim;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *HitWallAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *WasParriedAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *BlockedAttackAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *ChamberAnim;

    void ResetComboCount() { CurrentCombo = 0; }
    int GetComboCount() const { return CurrentCombo; }
    void IncreaseComboCount() { ++CurrentCombo; }

protected:
    /// must implement in BP
    // sets buffs/debuffs and spawn default weapon for unarmed stance
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    void OnStanceActivated();
    // resets buffs/debuffs and destroys default weapons (check if spawned it in the first place)
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    void OnStanceDeActivated();
    ///

    UCombatComponent* GetCombatComp() const;
    virtual void UpdateOwnerAnims(const UCombatStance* DefaultStance);
    // used for req checks, set when gets stance resolved
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    ACharacterCreature* Owner = nullptr;

    int CurrentCombo = 0;
};


UCLASS(BlueprintType, Blueprintable)
class UMeleeCombatStance : public UCombatStance
{
    GENERATED_BODY()

public:
    UMeleeCombatStance();

    void StanceSwitched(ACharacterCreature* NewOwner, const UCombatStance* DefaultStance) override;
    void ResetStance() override;
    bool IsStanceValid(const ACharacterCreature* NewOwner) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsParryAllowed = true;
    UPROPERTY(EditAnywhere)
    float ParryStaminaCost = 15;
};


UCLASS(BlueprintType, Blueprintable)
class URangeCombatStance : public UCombatStance
{
    GENERATED_BODY()

public:
    bool IsStanceValid(const ACharacterCreature* NewOwner) const override;

protected:
    void UpdateOwnerAnims(const UCombatStance* DefaultStance) override;
};
