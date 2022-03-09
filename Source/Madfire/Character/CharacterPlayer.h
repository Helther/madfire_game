#pragma once

#include "CharacterCreature.h"

#include "CharacterPlayer.generated.h"

class UCameraComponent;
class USpringArmComponent;


//состояние персонажа с точки зрения режима контроллера
enum class EPawnGameplayState
{
    InWorld = 0,
    InTrade,
    InDialogue,
    InInventory,
    InMenu,
    InContextMenu,
    InMountMode,

    MAX    //dlc stuff
};

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
    // temp value for min input amount to register
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementDeadZone;

protected:
    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

    virtual void OnWalkingOffLedge_Implementation(const FVector &PreviousFloorImpactNormal,
                                       const FVector &PreviousFloorContactNormal,
                                       const FVector &PreviousLocation,
                                       float TimeDelta) override;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraDistanceDefault;

    // текущий режим контроллера чарактера
    EPawnGameplayState CurrentGameplayState;
    // in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NewDirectionTime; 

private:
    EControlDirections GetInputDirection();
    void OnDirectionTimeout(uint8 directions);
    bool IsActionKeyPressed(const FName& ActionKey);

    //движение вперед-назад
    void MoveForward(float AxisValue);
    //вправо-влево
    void MoveRight(float AxisValue);
    //камера вверх-вниз
    void CameraPitch(float AxisValue);
    //камера влево-вправо
    void CameraYaw(float AxisValue);

    //вызывается при нажатии на бег-шаг
    void RunAction();
    //вызывается при нажатии на спринт
    void SprintAction();
    //при отпускании спринт
    void UnSprintAction();
    // вызывается при нажатии на присяд
    void CrouchAction();
    //when jump button pressed
    void JumpAction();
    //when jump button released
    void UnJumpAction();
    //jump button held
    void JumpRepAction();
    // нажатие кнопки уклонения
    void DodgeAction();
    // нажатие/отпускание кнопки атака
    void AttackOnAction();
    void AttackOffAction();
    // нажатие/отпускание кнопки блок
    void BlockOnAction();
    void BlockOffAction();
    // нажатие кнопки достать/спрятать оружие
    void WeaponAction();
    // нажатие кнопки взаимодействия
    void InteractAction();
    // activate current ability
    void AbilityOn();
    // stop activating current ability
    void AilityOff();

    void CameraDistanceIn();
    void CameraDistanceOut();
    // new direction timer
    FTimerHandle FaceTimer;
    FTimerHandle SideTimer;
    bool FaceGotOld = false;
    bool SideGotOld = false;
};
