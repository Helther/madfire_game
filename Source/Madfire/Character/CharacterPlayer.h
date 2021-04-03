// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Character/CharacterBase.h"

#include "CharacterPlayer.generated.h"

UENUM(BlueprintType, meta = (Bitflags))
// Character Control Bit mask with values as priorities (the more the better)
enum ECCState
{
    NoCC          = 0,
    Sleep         = 1 UMETA(DisplayName = "Sleeping"),
    LossOfBalance = 2 UMETA(DisplayName = "LostAllBalance"),
    Stun          = 4 UMETA(DisplayName = "Stunned"),
    KnockDown     = 8 UMETA(DisplayName = "KnockedDown"),
    Stasis        = 16 UMETA(DisplayName = "UnderStasis")
};
ENUM_CLASS_FLAGS(ECCState)

/**
 * 
 */
UCLASS()
class MADFIRE_API ACharacterPlayer : public ACharacterBase
{
    GENERATED_BODY()

public:
    ACharacterPlayer(const FObjectInitializer &ObjectInitializer);

    void OnJumped_Implementation() override;

    UFUNCTION(BlueprintCallable, Category = "Character Base") // todo debug
    void StartAnimTest(float InPlayRate);

    void StartAnimation(UAnimMontage *AnimMontage, const FOnMontageBlendingOutStarted &EndCallBack, float InPlayRate, FName StartSectionName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Character Base")
    void InterruptAnimation(ECCState Source, int OldState);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "ECCState"), Category = "Character Stats")
    int CurrentCCState;

    UPROPERTY(BlueprintReadWrite, Category = "Character Components", VisibleAnywhere)
    UCombatComponent *CombatComponent;

    UBaseMovementComponent *GetMovementComponent() const override;

protected:
    // Called to bind functionality to input
    virtual void
      SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

    virtual void
      OnWalkingOffLedge_Implementation(const FVector &PreviousFloorImpactNormal,
                                       const FVector &PreviousFloorContactNormal,
                                       const FVector &PreviousLocation,
                                       float TimeDelta) override;

    // clang-format off
    UFUNCTION(BlueprintCallable, Category = "Animation")
        void CCStunEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION(BlueprintCallable, Category = "Animation")
        void CCKnockDownEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION(BlueprintCallable, Category = "Animation")
        void CCStatisEnded(UAnimMontage *animMontage, bool bInterrupted);
    // todo add other

    int GetHighestPriorityCC(int bitmask);
    void PostInitializeComponents() override;

    TMap<ECCState, void (ACharacterPlayer::*)(UAnimMontage *, bool)> CCInterruptDelegates;
    TMap<ECCState, UAnimMontage *> CCAnims;
};
