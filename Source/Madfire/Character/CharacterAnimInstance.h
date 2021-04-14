// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"

#include "CharacterAnimInstance.generated.h"

// names of animation slots which used to play one anim at a time
struct FAnimSlotNames
{
    static const FName FullBody;
    static const FName UpperBody;
    static const FName Head;
    static const FName RightHand;
    static const FName LeftHand;
};

// name of groups of anim slots to potentialy interrupt several animations in
// different slots at one go
struct FAnimGroupNames
{
    static const FName MainGroup;
};

UENUM(BlueprintType)
// Animation classification types with values as priorities
enum class AnimTypes : uint8
{
    Block     = 0 UMETA(DisplayName = "Block"),
    Parkour   = 1 UMETA(DisplayName = "Parkour"),
    SpellCast = 2 UMETA(DisplayName = "SpellCast"),
    Attack    = 3 UMETA(DisplayName = "Attack"),
    Dodge     = 4 UMETA(DisplayName = "Dodge"),
    CC        = 5 UMETA(DisplayName = "Character Control")
};

UCLASS()
class MADFIRE_API UCharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacterBase *Character;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *SleepAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *LossOfBalanceAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *StunAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *KnockDownAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *StasisAnim;

private:
    void NativeUpdateAnimation(float DeltaSeconds) override;
};
