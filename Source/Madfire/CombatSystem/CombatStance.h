#pragma once

#include "ComboFSM.h"
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/BlendSpace.h"
    
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
    Crouch,
    CounterAttack
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
    bool IsBlockable;
    UPROPERTY(EditAnywhere)
    bool IsParriable;
    UPROPERTY(EditAnywhere)
    bool IsInterruptable;
    UPROPERTY(EditAnywhere)
    ESpecialType Special;

    // Delegate for special on hit action
    UPROPERTY(BlueprintAssignable)
    FCustomAttackEvent CustomOnHitEvent;
};

USTRUCT(BlueprintType)
struct FChargeAttackIngo
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
    TArray<FChargeAttackIngo> ChargeAttackStages;
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
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    // TODO add custom aim offset blend space for ranged
    bool IsMelee = true;
    ////////////////////////// combat mode speeds//////////////////////////
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Speeds")
    float MaxCombatRunSpeed{};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Speeds")
    float MaxCombatCrouchedSpeed{};

    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Combat Speeds")
    float MaxCombatSprintSpeed{};
    // Max move speed during blocking
    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Combat Speeds")
    float MaxBlockSpeed{};

    UPROPERTY(EditAnywhere)
    float ParryStaminaCost = 15;
    // used for req checks, set when gets stance resolved
    UPROPERTY(BlueprintReadOnly)
    class ACharacterBase* Owner = nullptr;

    // Implement This, it's really important!
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Stance Interface")
    bool CheckRequirement(class ACharacterBase *character);
    
    // use size as max combo count
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FComboInfo> ComboAttacks;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAttackInfo> SpecialAttacks;
    // custom equipment specific animations for unique stances
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage *CustomBlockAnim;// TODO modify anim speed according to shield type after stance resolve
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage *CustomParryAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UBlendSpace *CustomStandLocomotion;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UBlendSpace *CustomCrouchLocomotion;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage *CustomDrawAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage *CustomSheathAnim;

    UBlendSpace *CurrentCrouchLocomotion;
    UBlendSpace *CurrentStandLocomotion;
    UAnimMontage *CurrentBlockAnim;
    UAnimMontage *CurrentParryAnim;
    UAnimMontage *CurrentDrawAnim;
    UAnimMontage *CurrentSheathAnim;
    
    // TODO for testing
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int CurrentCombo = 0;

};
