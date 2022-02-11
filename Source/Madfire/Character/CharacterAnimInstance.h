#pragma once

#include "CharacterAnimInstanceBase.h"
#include "CoreMinimal.h"

#include "CharacterAnimInstance.generated.h"



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
    void OnCollisionActivation(bool mainHand, bool activate);

    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnParriableActivation(bool activate);

    UFUNCTION(BlueprintCallable, Category = "Anim Notify Interface")
    void OnDodgeInvulnerable(bool activate);

    UFUNCTION(BlueprintCallable)
    bool IsMovementAllowed() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *HitWallAnim;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *WasParriedAnim;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *BlockedAttackAnim;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC Anims")
    UAnimMontage *ChamberAnim;

private:
    void NativeUpdateAnimation(float DeltaSeconds) override;

};
