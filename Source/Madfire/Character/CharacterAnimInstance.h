#pragma once

#include "CharacterAnimInstanceBase.h"
#include "../CombatSystem/CombatStance.h"
#include "CoreMinimal.h"

#include "CharacterAnimInstance.generated.h"


// Used for anims with loop sections
struct FAnimSectionNames
{
    static const FName Start;
    static const FName End;
    static const FName Loop;
    static const FName Cancel;
};


UCLASS()
class MADFIRE_API UCharacterAnimInstance : public UCharacterAnimInstanceBase
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacterCreature *CharacterInstance;

    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnWeaponSwitch();

    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnAmmoGrab();


    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnAmmoAttach();

    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnProjectileFired();

    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnBlockActivation(bool activate);
    
    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnParryActivation(bool activate);
    // In combo this event should be set on the second frame of end section, not earlier
    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnMovementAllowed(bool activate);
    
    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnRotationAllowed(bool activate);

    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnComboCanContinue();

    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnCollisionActivation(EArmsSlot WeaponSlot, bool activate);

    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnParriableActivation(bool activate);

    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnDodgeInvulnerable(bool activate);

    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnCounterChamber(bool activate);

    UFUNCTION(BlueprintCallable)
    bool IsMovementAllowed() const;

    UPROPERTY(BlueprintReadOnly, Category = "CC Anims")
        UAnimMontage* CurrentHitWallAnim;
    UPROPERTY(BlueprintReadOnly, Category = "CC Anims")
        UAnimMontage* CurrentWasParriedAnim;
    UPROPERTY(BlueprintReadOnly, Category = "CC Anims")
        UAnimMontage* CurrentBlockedAttackAnim;
    UPROPERTY(BlueprintReadOnly, Category = "CC Anims")
        UAnimMontage* CurrentChamberAnim;

    UPROPERTY(BlueprintReadOnly, Category = "Animations")
        UBlendSpace* CurrentCrouchLocomotion;
    UPROPERTY(BlueprintReadOnly, Category = "Animations")
        UBlendSpace* CurrentStandLocomotion;
    UPROPERTY(BlueprintReadOnly, Category = "Animations")
        UAimOffsetBlendSpace* CurrentAimOffset;
    UPROPERTY(BlueprintReadOnly, Category = "Animations")
        UAnimMontage* CurrentBlockAnim;
    UPROPERTY(BlueprintReadOnly, Category = "Animations")
        UAnimMontage* CurrentParryAnim;
    UPROPERTY(BlueprintReadOnly, Category = "Animations")
        UAnimMontage* CurrentDrawAnim;
    UPROPERTY(BlueprintReadOnly, Category = "Animations")
        UAnimMontage* CurrentSheathAnim;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
        TArray<UCombatStance*> DefaultStances;

private:
    void NativeUpdateAnimation(float DeltaSeconds) override;

};
