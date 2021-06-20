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
    class ACharacterPlayer *CharacterPlayer;

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

    UFUNCTION(BlueprintCallable)
    bool IsMovementAllowed() const;

private:
    void NativeUpdateAnimation(float DeltaSeconds) override;

};
