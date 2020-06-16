// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

struct FInputKeyNames
{
    static const FName MoveForward;
    static const FName MoveRight;
    static const FName Run;
    static const FName Sprint;
    static const FName Crouch;
    static const FName Jump;
    static const FName CameraPitch;
    static const FName CameraYaw;
    static const FName ClimbDownTest;
};

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

	MAX //dlc stuff
};

/**
 * 
 */
UCLASS()
class MADFIRE_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
    bool IsActionKeyPressed(const FName &ActionKey);
private:
    // текущий режим контроллера чарактера
    EPawnGameplayState CurrentGameplayState;
/*
	UPROPERTY(EditAnywhere, Category = "Character Input")
		float PitchSensitivity;
	UPROPERTY(EditAnywhere, Category = "Character Input")
		float YawSensitivity;
*/

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
	// нажатие кнопки атака
	void AttackAction();
	// нажатие кнопки блок
	void BlockAction();
	// нажатие кнопки уклонения
	void DodgeAction();
	// нажатие кнопки достать/спрятать оружие
	void WeaponAction();
	// нажатие кнопки взаимодействия
	void InteractAction();

	class ACharacterPlayer* Player;

protected:
	ABasePlayerController();

	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	
};

