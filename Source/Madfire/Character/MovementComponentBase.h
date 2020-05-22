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
	CanSprint = 32	        UMETA(DisplayName = "CanSprint"),
    CanCrouch = 64          UMETA(DisplayName = "CanCrouch")
};
ENUM_CLASS_FLAGS(EMoveAbilities)

struct StartClimbData
{
    FVector Location;
    FRotator Rotation;
};

/**
 * 
 */
UCLASS()
class MADFIRE_API UBaseMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UBaseMovementComponent(const class FObjectInitializer &ObjectInitilizer);

	/////////////////////////////////////////////////////
	/////////////////////====== Speeds ==========/////////////////
    

	//////////Default max speeds
	///////////////////////Non combat////////////////////////
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
    
    void CalculateMaxCombatSpeeds(float SpeedModifier);
    void CalculateMaxSpeeds(float SpeedModifier);
    void CalculateMaxBlockingSpeed(float SpeedModifier);
    void CalculateMaxCastingSpeed(float SpeedModifier);

protected:
    // для обновления максимальныз скоростей


	/////////////////////// max ue4 current speeds //////
	//MaxWalkSpeed
	//MaxWalkSpeedCrouched
	//MaxSwimSpeed
	//MaxFlySpeed
	//MaxCustomMovementSpeed

	//Walk mode max current speeds
	float MaxRunSpeed{};
	float MaxSprintSpeed{};
	float MaxRunSpeedCrouched{};


	////////////////=========== Speeds end =========/////////////////

	/////////////////======= Fall damage ========//////////////////
public:
    bool bFallDamageInvul{false}; // блокировка урона от падения
    //высота падения при которой начинает наносится дамаг
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fall Damage")
        float fallHeightThreshold{};
    //высота падения при которой урон максимален
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fall Damage")
        float MaxfallHeightThresholdBase{};

private:
     //высота персонажа в момент потери контакта с поверхностью
	float fallLocationHeight{false};
    //проверка на получение урона от падения
	bool FallDamageCheck();
    //нанесение урона от падения
	void ApplyFallDamage(float FallDistance);
	
	/////////////////======= Fall damage END ========//////////////////

    ////////////////////////=========Parkour shit========/////////////////////////
public:
    // максимальное отклонение направления чарактера относительно нормали стены в градусах
    UPROPERTY(EditAnywhere, Category = "Climb general")
        float MaxClimbRotationAngle;
    // доп смещение от стены при старте анимации
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb general")
        float LocationHorizOffset = 0;
    // same day different axis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb general")
        float LocationVertOffset = 0;
private:
    // проверка если угол относительно стены и перса не превышает максимум
    bool checkRelativeRotation(const float WallNormalYaw);
    // check if there is enough space for character capsule before teleport for anim,
    // where location is the center of the capsule
    EClimbTraceResult IsEnoughSpace(const FVector& StartLocation,
                                    const FVector& EndLocation = FVector(0,0,0),
                                    const ECollisionChannel &startCC =
                                        ECollisionChannel::ECC_WorldStatic,
                                    const ECollisionChannel &endCC = EWallChannel);
    // check if character playing root motion or teleporting
    bool IsRootOrTeleport();

    // does trace in character's capsule shape для проверки на лаличие достаточного места
    bool IsCharacterCapsuleOk(const FVector& centerLoc, const bool& crouching,
                              const bool &H2L = true, const ECollisionChannel &CC =
                                ECollisionChannel::ECC_WorldStatic) const;
  
    //////////////////////////////Wall Climb//////////////////////////////
public:
    // проверяет что персонаж упал ниже нужной величины прежде чем повторно зацепиться
    bool CheckCanHangAgain();
    // min distance between character and ground underneath allowed
    // MinClimbWallHeight = Character->GetDefaultHalfHeight() / 2;
    ////////// montages
    // монтаж залезания на стену с присядом в конце
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Wall")
        UAnimMontage *ClimbWallCrouchedMontage;
    // монтаж залезания на стену стоя
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Wall")
        UAnimMontage *ClimbWallMontage;

    //check if there is a  climbable wall
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
    bool bCanHangAgain{true}; // тру если чарактер может повиснуть на стене
    float CanHangAgainDistance; //todo временное значение пути падения для повтора повиса
    // возвращает локацию или ротатор для повисания персонажа рассчитанную
    // с учетом размеров его капсулы
    FVector FindHangLoc(const FHitResult &HitH, const FHitResult &HitV) const;
    FRotator FindHangRot(const FHitResult &HitH) const;


	//////////////////////////Ledge Climb///////////////////////////

public:
    // макс размер или длина препятствия в горизонтальной плоскости
    // на которой можно сделать vault
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
    //horizontal length of the trace
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Ledge")
        float LedgeTraceLength{};

    //проверка препятствия на vault  и climb, 
    //если low = true высота от ступеньки до пояса, иначе от пояса до макушки
    bool CanVaultOrClimb(bool IsLow);

private:
    //телепорт к нужной локации и установка ориентации чарактера,
    // активация монтажа vault, включение режима climbLedge
    bool DoVault(const StartClimbData& locationData, bool IsLow);
    //телепорт к нужной локации и установка ориентации чарактера,
    // активация монтажа climb, включение режима climbLedge
    bool DoClimbLedge(const StartClimbData& locationData, 
                        bool IsLow);
    //проверка на попадание стены в нужный диапазон высот
    bool VaultSpaceCheck(const StartClimbData& LedgeData);
    //поиск оптимальной точки на стене для начала анимации клаймб паркура
    //(отклонения высоты относительно дна капсулы)
    TSharedPtr<StartClimbData> GetClimbStartLoc(const FTraceSpawnParams& startTrace,const float& minHeight = 0,
                             const float& maxHeight = 0);
    FTraceParams params;
    FTraceSpawnParams GetHorizTraceSpawnParams(bool IsLow);

    FTraceSpawnParams GetVertTraceSpawnParams(const FHitResult& Hit, bool IsLow);

    ////Ladder Climb


	////Slide

    ////Dodge



	////////////////////===========Parkour shit END============/////////////////////////

    //////////////////=========== Общие вспомогательные =======/////////////////////////
public:
    void Update(); ///todo

	bool bWantToSprint{false}; //тру если пытается перейти на спринт
	bool bIsRunning{false}; // тру если персонаж в режима бега
	bool bIsSprinting{false}; // тру если персонаж в режиме спринта
	bool bIsTeleporting{false}; // тру если сейчас телепортируется
    // false если перс перегружен или замедлен до остановки 
    bool CanMove(); 
    //todo для использования таких масок в блупринтах приходится создавать местную переменную там
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EMoveAbilities"),
            Category = "Character Movement")
		uint8 MoveAbilitiesDefault;
    uint8 MoveAbilitiesCurrent;
protected:
    /// наследованный мусор
    ///
    ///
	void OnMovementModeChanged(EMovementMode, uint8) override;
	float GetMaxSpeed() const override;
    //tick functions exlusively for different movement modes
    void PhysWalking(float deltaTime, int32 Iterations) override;
	void PhysFalling(float deltaTime, int32 Iterations) override;
    void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void NotifyJumpApex() override;
    virtual void Crouch(bool bClientSimulation = false) override;
    virtual bool DoJump(bool bReplayingMoves) override;
	//From unreal engine's Uobject
	void BeginPlay() override;
	void TickComponent(float DeltaTime,
                        enum ELevelTick TickType,
                        FActorComponentTickFunction *ThisTickFunction) override;

private:
    class ACharacterBase* Character;
};
