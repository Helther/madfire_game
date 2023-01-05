#pragma once

#include "CharacterCreature.h"
#include "CoreMinimal.h"
#include "CharacterPlayer.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UPlayerPawnCameraComponent;

UENUM(BlueprintType)
enum class EGameplayReturnState : uint8
{
    InWorld,
    InTrade,
    InDialogue,
    InInventory,
    InMenu,
    InContextMenu,
    InMountMode,

    MAX  //UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EKeyState : uint8
{
    Pressed,
    Released
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharacterInput, EGameplayReturnState, ReturnState, EKeyState, KeyState);

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
    void SetAbilityCameraMode(TSubclassOf<UCameraMode> CameraMode) override;
    void ClearAbilityCameraMode() override;
    UFUNCTION()
    void SetSprintCameraMode(bool activate);

    UFUNCTION(BlueprintCallable, Category = "Character Animation")
    void animTest(UAnimMontage* AnimMontage) {StartAnimation(AnimMontage, FOnMontageBlendingOutStarted(),1); }

    UFUNCTION(BlueprintImplementableEvent, Category = "Interact", Meta = (ExpandEnumAsExecs = "Branches"))
    void OnInteract(EGameplayReturnState Branches, EKeyState KeyState);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Components")
    UPlayerPawnCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Components")
    USpringArmComponent* SpringArmComponent;
    
    void UseAimComponent(class UAbilityAimComponent *) override;

    void AdjustCameraZoom(float Offset);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraZoomStep;

protected:
    void BeginPlay() override;
    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

    virtual void OnWalkingOffLedge_Implementation(const FVector &PreviousFloorImpactNormal,
                                       const FVector &PreviousFloorContactNormal,
                                       const FVector &PreviousLocation,
                                       float TimeDelta) override;

};
