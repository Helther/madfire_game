#pragma once

#include "CharacterCreature.h"

#include "CharacterPlayer.generated.h"

class UCameraComponent;
class USpringArmComponent;

/**
 * 
 */
UCLASS()
class MADFIRE_API ACharacterPlayer : public ACharacterCreature
{
    GENERATED_BODY()

public:
    ACharacterPlayer(const FObjectInitializer &ObjectInitializer);

    void OnJumped_Implementation() override;

    UFUNCTION(BlueprintCallable, Category = "Character Animation")
    void animTest(UAnimMontage* AnimMontage) {StartAnimation(AnimMontage, FOnMontageBlendingOutStarted(),1); }

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
    void OnInteract();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Components")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Components")
	USpringArmComponent* SpringArmComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraDistanceMin;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraDistanceMax;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraZoomStep;

protected:
    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

    virtual void OnWalkingOffLedge_Implementation(const FVector &PreviousFloorImpactNormal,
                                       const FVector &PreviousFloorContactNormal,
                                       const FVector &PreviousLocation,
                                       float TimeDelta) override;

    float CameraDistanceDefault;
};
