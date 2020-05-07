#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "./TraceFunctional/TraceFunctional.h"
#include "MovementComponentBase.generated.h"

UENUM(BlueprintType)

enum ECustomMovementMode
{
	ClimbingWall = 0, //Повисание на стене
	ClimbingLadder, //Лазаие по лестнице
	ClimbingLedge, //Перелезание через невысокое препядствие
	Slide, //подкат
	Dodge // уклонение
};

//possible results of wall climbing check for available space
enum EClimbTraceResult
{
    SuccessStand = 0,
    SuccessCrouch,
    FailAtStart,
    FailAtEnd
};

UENUM(BlueprintType, meta = (Bitflags))
enum EMoveAbilities     //!< Флаги возможностей передвижения.
{
	CanJump = 1			    UMETA(DisplayName = "CanJump"),
	CanClimb = 2		    UMETA(DisplayName = "CanClimb"),
	CanLadder = 4			UMETA(DisplayName = "CanLadder"),
	CanSlide = 8			UMETA(DisplayName = "CanSlide"),
	CanDodge = 16			UMETA(DisplayName = "CanDodge"),
	CanSprint = 32	        UMETA(DisplayName = "CanSprint")
};
ENUM_CLASS_FLAGS(EMoveAbilities)

/**
 * 
 */
UCLASS()
class MADFIRE_API UBaseMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UBaseMovementComponent(const class FObjectInitializer &ObjectInitilizer);

	void NotifyJumpApex() override;
	bool bWantToSprint{}; //тру если пытается перейти на спринт
	bool bIsRunning{}; // тру если персонаж в режима бега
	bool bIsSprinting{}; // тру если персонаж в режиме спринта
	bool bIsTeleporting{}; // тру если сейчас телепортируется

    //todo добавить проверки  на возможность перед перед действиями в том числе стандартными
    //todo для использования таких масок в блупринтах приходится создавать местную переменную там
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EMoveAbilities"), Category = "General")
		uint8 MoveAbilities;

	/////////////////////////////////////////////////////
	///////////////////////////////Speeds/////////////////
    
    // для обновления максимальныз скоростей
	void CalculateMaxSpeed(float SpeedModifier); 
	//////////Default max speeds
	//////Non combat
    //базовая скорость бега
	UPROPERTY(EditAnywhere, Category = "Character Default Speeds")
        float MaxRunSpeedDefault{};
    //базовая скорость ходьбы
	UPROPERTY(EditAnywhere, Category = "Character Default Speeds")
        float MaxWalkSpeedDefault{};
    //базовая скорость спринта
	UPROPERTY(EditAnywhere, Category = "Character Default Speeds")
        float MaxSprintSpeedDefault{};
    //базовая скорость движения вдоль стены
	UPROPERTY(EditAnywhere, Category = "Character Default Speeds")
        float MaxWallClimbSpeedDefault{};
    //базовая скорость ходьбы в присяде
	UPROPERTY(EditAnywhere, Category = "Character Default Speeds")
        float MaxWalkSpeedCrouchedDefault{};
    //базовая скорость бега в присяде
	UPROPERTY(EditAnywhere, Category = "Character Default Speeds")
        float MaxRunSpeedCrouchedDefault{};
    //базовая скорость плавания
	UPROPERTY(EditAnywhere, Category = "Character Default Speeds")
        float MaxSwimSpeedDefault{};
    //базовая скорость полета
	UPROPERTY(EditAnywhere, Category = "Character Default Speeds")
        float MaxFlySpeedDefault{};


	////////////////////////////max default speeds//////////////////
	//max walk speeds
	////Unreal created speeds:
	//MaxWalkSpeed
	//MaxWalkSpeedCrouched
	//MaxSwimSpeed
	//MaxFlySpeed
	//MaxCustomMovementSpeed

	//Walk mode max current speeds
	float MaxRunSpeed{};
	float MaxSprintSpeed{};
	float MaxRunSpeedCrouched{};
								  
/*
	//combat
	UPROPERTY(EditAnywhere, Category = "Character Speeds")
		float MaxCombatRunSpeed;

	UPROPERTY(EditAnywhere, Category = "Character Speeds")
		float MaxCombatWalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Character Speeds")
		float MaxCombatSprintSpeed;

	UPROPERTY(EditAnywhere, Category = "Character Speeds")
        float MaxCastSpeed; //Max move speed during cast
	
    UPROPERTY(EditAnywhere, Category = "Character Speeds")
        float MaxBlockSpeed; //Max move speed during blocking

	//////////////////////Speeds end//////////////////
*/

	////////////////////Fall damage//////////////////
public:
    bool bFallDamageInvul{}; // блокировка урона от падения
    //высота падения при которой начинает наносится дамаг
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fall Damage")
        float fallHeightThreshold{};
    //высота падения при которой урон максимален
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fall Damage")
        float MaxfallHeightThresholdBase{};

private:
	float fallLocationHeight{}; //высота персонажа в момент потери контакта с поверхностью
	bool FallDamageCheck();	//проверка на получение урона от падения

	void ApplyFallDamage(float FallDistance); //нанесение урона от падения
	

    ////////////////////////=========Parkour shit========/////////////////////////
public:
    // максимальное отклонение направления чарактера относительно нормали стены в градусах
    UPROPERTY(EditAnywhere, Category = "Climb general")
        float MaxClimbRotationAngle;

private:
    //check if there is enough space for character capsule before teleport for anim, where location is the center of the capsule
    EClimbTraceResult IsEnoughSpace(const FVector& StartLocation,
                                    const FVector& EndLocation = FVector(0,0,0),
                                    const ECollisionChannel &startCC = ECollisionChannel::ECC_WorldStatic,
                                    const ECollisionChannel &endCC = EWallChannel);
    //check if character playing root motion or teleporting
    bool IsRootOrTeleport();

    //does trace in character's capsule shape
    bool IsCharacterCapsuleOk(const FVector& centerLoc, const bool& crouching,
                              const bool &H2L = true, const ECollisionChannel &CC =
                                ECollisionChannel::ECC_WorldStatic) const;
  
    //////////////////////////////Wall Climb//////////////////////////////
public:
    bool bCanHangAgain{}; // тру если чарактер может повиснуть на стене

    // min distance between character and ground underneath allowed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Wall")
        float MinClimbWallHeight{};
    // offset value in direction opposite to the wall
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Wall")
        float HangLocationHorizOffset = 1;
    // same day different axis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Wall")
        float HangLocationVertOffset = 0;
    ////////// montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Wall")
        UAnimMontage *ClimbWallAnimStand;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Wall")
        UAnimMontage *ClimbWallAnimCrouched;
    // монтаж залезания на стену
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Wall")
        UAnimMontage *ClimbWallMontage;

    //check if there is a  climbababe wall
    void IsClimbableWall(FHitResult HitWallH, bool CheckCharacterRotation);
    //allows to get on top of wall while hanging
    void ClimbWall();
    //sets falling movement mode when pressed crouch while climbing
    void GetOffWall();
    //climb down and hang on the wall which character just walked off from
    void ClimbDownWall();
    // hang on wall while in air
    void HangOnWall();
    // jump from wall while hanging
    void JumpOffWall();

private:

    // возвращает локацию или ротатор для повисания персонажа рассчитанную
    // с учетом размеров его капсулы
    FVector FindHangLoc(const FHitResult &HitH, const FHitResult &HitV) const;
    FRotator FindHangRot(const FHitResult &HitH) const;


	//////////////////////////Ledge Climb///////////////////////////

public:
    // наибольшая высота препятствия относительно днища персонажа
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        float MaxClimbLedgeHeight{}; /// приравнена к половине высоты капсулы
    //макс ширина препятствия в горизонтальной плоскости на которой можно сделать vault
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        float MaxVaultLedgeSize{};
    //монтаж перелезания
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        UAnimMontage *VaultMontageLow;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        UAnimMontage *VaultMontageHigh;
    // монтаж залезания на выступ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        UAnimMontage *ClimbLedgeMontageLow;
    // монтаж залезания на выступ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        UAnimMontage *ClimbLedgeMontageHigh;
    // монтаж залезания на выступ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        UAnimMontage *ClimbLedgeCrouchedMontageLow;
    // монтаж залезания на выступ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        UAnimMontage *ClimbLedgeCrouchedMontageHigh;

    //trace params
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        FVector HorizLedgeTraceOffset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        float LedgeTraceLength{}; //horizontal length of the ray
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        FVector VerticalLedgeTraceOffset{};
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        float LedgeTraceHeight{}; //vertical distance for the trace from center of the character
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        float LedgeTraceWidth{};
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        float LedgeTraceRadius{};

    //проверка препятствия на vault  и climb, 
    //если low = true высота от ступеньки до пояса, иначе от пояса до макушки
    bool CanVaultOrClimb(const bool &IsLow);

private:

    //телепорт к нужной локации и установка ориентации чарактера,
    // активация монтажа vault, включение режима climbLedge
    bool DoVault(const FHitResult &HorizHit,const FHitResult &VertHit, const bool& IsLow);
    //телепорт к нужной локации и установка ориентации чарактера,
    // активация монтажа climb, включение режима climbLedge
    bool DoClimbLedge(const FHitResult &HorizHit,const FHitResult &VertHit, const bool& IsLow);
    //проверка на попадание стены в нужный диапазон высот
    bool LedgeHeightCheck(const float WallZHeight, const float charAndWallAngle, const bool& IsLow);
    //наличие места над узким препядствием
    bool VaultSpaceCheck(const FHitResult& HorizHit,const FHitResult& VertHit);
    //поиск оптимальной точки на стене по горизонтали для клаймб паркура
    //(отклонения высоты относительно дна капсулы)
    FVector GetClimbStartLoc(const FTraceSpawnParams& startTrace,const float& maxHeight = 0,
                             const float& minHeight = 0);

    FTraceSpawnParams GetHorizTraceSpawnParams(const bool& IsLow);

    FTraceSpawnParams GetVertTraceSpawnParams(const FHitResult& Hit, const bool& IsLow);

    ////Ladder Climb


	////Slide

    ////Dodge



	////////////////////===========Parkour shit END============/////////////////////////

    class ACharacterBase* Character;//todo поменять на класс игрового перса
public:
    void Update(); ///todo
protected:
    /// наследованный мусор
    ///From CharacterMovementComponent
    ///
	void OnMovementModeChanged(EMovementMode, uint8) override;
	float GetMaxSpeed() const override;
    //tick functions exlusively for different movement modes
    void PhysWalking(float deltaTime, int32 Iterations) override;
	void PhysFalling(float deltaTime, int32 Iterations) override;
    void PhysCustom(float deltaTime, int32 Iterations) override;
	//From unreal engine's Uobject
	void BeginPlay() override;
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};
