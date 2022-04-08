#pragma once

#include "ComboFSM.h"
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/BlendSpace.h"
#include "../Rules/GameRules.h"
    
#include "CombatStance.generated.h"


UENUM(BlueprintType, meta = (Bitflags))
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomAttackEvent, AActor *, Actor);

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


UCLASS(BlueprintType, Blueprintable, EditInlineNew)
class MADFIRE_API UCombatStance : public UDataAsset
{
    GENERATED_BODY()

public:
    FAttackInfo* GetAttackInfoForSpecial(ESpecialType type);
    
    // Need weapon in each hand to use (exludes weapon+shield combination)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsTwoWeapon = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsBlockAllowed = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsParryAllowed = true;
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

    UPROPERTY(EditAnywhere)
    float ParryStaminaCost = 15;
    // TODO do we need to set it?
    // used for req checks, set when gets stance resolved
    UPROPERTY(BlueprintReadOnly)
    class ACharacterCreature* Owner = nullptr;

    // Implement This, it's really important!
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Stance Interface")
    bool CheckRequirement(ACharacterCreature*character) const;

    bool CheckRequirement_Implementation(ACharacterCreature* character) const { return true; }
    
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
};
