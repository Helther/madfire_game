#pragma once

#include "Character/CharacterAnimInstance.h"
#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TraceFunctional/TraceFunctional.h"

#include "MovementComponentBase.generated.h"


enum EControlDirections
{
    ForwardD = 0,
    BackD,
    RightD,
    LeftD,
    FLeftD,
    FRightD,
    BLeftD,
    BRightD
};

struct FTransitionSectionNames
{
    static const FName Start;
    static const FName End;
};

USTRUCT(BlueprintType)
struct FMoveStaminaCosts
{
    GENERATED_BODY()
    // отрицательные т.к. прибавляем при потреблении
    UPROPERTY(EditAnywhere)
    float Jump = -20;
    UPROPERTY(EditAnywhere)
    float Sprint = -5;
    UPROPERTY(EditAnywhere)
    float Climb = -20;
    UPROPERTY(EditAnywhere)
    float Vault = -15;
};

UENUM(BlueprintType)
enum ECustomMovementMode
{
    NoParkour = 0,
    ClimbingWall, //Повисание на стене
    ClimbingLedge, //Перелезание через невысокое препядствие
    Dodge // уклонение
    //ClimbingLadder, //Лазаие по лестнице
    //Slide, //подкат
};

// possible results of wall climbing check for available space
enum EClimbTraceResult
{
    SuccessStand = 0,
    SuccessCrouch,
    FailAtStart,
    FailAtEnd
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum EMoveAbilities //!< Флаги возможностей передвижения
{
    NoMove   = 0,
    CanJump  = 1 UMETA(DisplayName = "CanJump"),
    CanClimb = 2 UMETA(DisplayName = "CanClimb"),
    CanDodge  = 4 UMETA(DisplayName = "CanDodge"),
    CanSprint = 8 UMETA(DisplayName = "CanSprint"),
    CanCrouch = 16 UMETA(DisplayName = "CanCrouch")
    //CanLadder = 			UMETA(DisplayName = "CanLadder"),// cut content
    //CanSlide =     		UMETA(DisplayName = "CanSlide"),
};
ENUM_CLASS_FLAGS(EMoveAbilities)

// climb starting transform
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

    //Walk mode max current speeds
    float MaxRunSpeed{};
    float MaxSprintSpeed{};
    float MaxRunSpeedCrouched{};
    // blocks max run speed update inside this component
    bool maxSpeedBlock;
    // для обновления максимальных скоростей
    void CalculateMaxSpeeds(float SpeedModifier);
    float GetSpeedModifier() const;

protected:
    /////////////////////// max ue4 current speeds //////
    //MaxWalkSpeed
    //MaxWalkSpeedCrouched
    //MaxSwimSpeed
    //MaxFlySpeed
    //MaxCustomMovementSpeed



    ////////////////=========== Speeds end =========/////////////////

    /////////////////======= Fall damage ========//////////////////
public:
    // блокировка урона от падения
    bool bFallDamageInvul{ false };
    //высота падения при которой начинает наносится дамаг, пределы 0.75*base-1.25*base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params")
    float MinFallHeightThresholdBase{};
    //высота падения при которой урон максимален, пределы 0.75*base-1.25*base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params")
    float MaxfallHeightThresholdBase{};
    //максимальное отклонеие относительно базового значения ((1 -+ k)*base)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float FallDamageScale{};

protected:
    float MinFallHeightThreshold;
    float MaxfallHeightThreshold;
    //высота персонажа в момент потери контакта с поверхностью
    float fallLocationHeight{};
    //проверка на получение урона от падения
    bool FallDamageCheck();
    //нанесение урона от падения
    void ApplyFallDamage(float FallDistance);

    /////////////////======= Fall damage END ========//////////////////

    ////////////////////////=========Parkour shit========/////////////////////////
public:
    // максимальное отклонение направления чарактера относительно нормали стены в градусах
    UPROPERTY(EditAnywhere, Category = "Climb general")
    float MaxClimbRotationAngle{};
    // доп смещение от стены при старте анимации
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb general")
    float LocationHorizOffset = 0;
    // same day different axis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb general")
    float LocationVertOffset = 0;
    // check if character playing root motion or teleporting
    bool IsRootOrTeleport() const;

protected:
    FTraceParams params;
    // проверка если угол относительно стены и перса не превышает максимум
    bool checkRelativeRotation(const float WallNormalYaw);
    // check if there is enough space for character capsule before teleport for anim,
    // where location is the center of the capsule
    EClimbTraceResult
      IsEnoughSpace(const FVector &StartLocation,
                    const FVector &EndLocation       = FVector(0, 0, 0),
                    const ECollisionChannel &StartCC = ECollisionChannel::ECC_WorldStatic,
                    const ECollisionChannel &EndCC   = EWallChannel);


    // does trace in character's capsule shape для проверки на лаличие достаточного места
    bool IsCharacterCapsuleOk(
      const FVector &CenterLoc, const bool &Crouching, const bool &H2L = true,
      const ECollisionChannel &CC = ECollisionChannel::ECC_WorldStatic) const;

    //////////////////////////////Wall Climb//////////////////////////////
public:
    // тру если если чарактер может прыгать от стен
    bool bWallJumpAbility{ true };
    // тру если чарактер может повиснуть на стене
    bool bCanHangAgain{ true };
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Wall")
    UAnimMontage *ClimbDownMontage;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Wall")
    UAnimMontage *HangMontage;
    //allows to get on top of wall while hanging
    void ClimbWall();
    //sets falling movement mode when pressed crouch while climbing
    void GetOffWall();
    //climb down and hang on the wall which character just walked off from
    void ClimbDownWall();
    // телепорт к стене для повисания while in air
    void HangOnFall();
    // найти точку для повисания на стене
    void HangOnWall(TSharedPtr<StartClimbData> HangData, bool ClimbDown);
    // jump from wall while hanging
    void JumpOffWall(const EControlDirections &Directions);

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
    bool DoVault(const StartClimbData &LocationData, bool IsLow);
    //телепорт к нужной локации и установка ориентации чарактера,
    // активация монтажа climb, включение режима climbLedge
    bool DoClimbLedge(const StartClimbData &LocationData, bool IsLow);
    //проверка на попадание стены в нужный диапазон высот
    bool VaultSpaceCheck(const StartClimbData &LedgeData);
    //поиск оптимальной точки на стене для начала анимации клаймб паркура
    //(отклонения высоты относительно дна капсулы)
    TSharedPtr<StartClimbData> GetClimbStartLoc(const FTraceSpawnParams &StartTrace,
                                                const float &MinHeight = 0,
                                                const float &MaxHeight = 0,
                                                bool CheckAngle        = true);
    FTraceSpawnParams GetTraceSpawnParams(bool IsLow);
    // не тру если перс пытается висеть на узком выступе
    bool CheckWallGeometry(const StartClimbData &Data);
    ////Slide


    ////Dodge


    ////Ladder Climb

    ////////////////////===========Parkour shit END============/////////////////////////

    //////////////////=========== Общие вспомогательные =======/////////////////////////
public:
    //////////////////======= рпг параметры
    ////// базовые параметры передвижения задаваемые в редакторе, из которых при апдейте
    // расчитываются актуальные значения
    // параметры прыжка

    // отзывчиваость в воздухе, пределы 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float AirControlBase{};
    // сила прыжка, пределы 0.75*base-1.25*base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params")
    float JumpZVelocityBase{};
    // замедление в воздухе при отстутствии движения вперед, пределы 0.75*base-1.25*base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params")
    float BrakingDecelerationFallingBase{};
    // трение о воздух при движении, пределы 0.75*base-1.25*base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params")
    float FallingLateralFrictionBase{};
    // время на которое можно продлить возвышение прыжка, пределы 0-JumpMaxHoldTimeBase
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "0.0", UIMin = "0.0"))
    float JumpMaxHoldTimeBase{};
    // вертикальная скорость прыжка от стены, пределы 0.75*base-1.25*base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params")
    float JumpWallZVelocityBase{};
    // горизонтальная скорость прыжка от стены, пределы 0.75*base-1.25*base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params")
    float JumpWallXYVelocityBase{};

    // параметры наземного движения

    // макс. ускорение на земле, пределы 0.75*base-1.25*base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params")
    float MaxAccelerationBase{};
    // аналогично замедление, пределы 0.75*base-1.25*base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params")
    float BrakingDecelerationWalkingBase{};
    // сцепление на земле, пределы 0.75*base-1.25*base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params")
    float BrakingFrictionBase{};
    // макс отношение перегруза при котором можно двигаться, пределы 1-1.2*base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "1.0", UIMin = "1.0"))
    float MaxOverweightMoveFactorBase{};
    //скорость вопроизведения анимаций паркура, пределы 0.75-1.25
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params")
    float AnimPlayRateBase{};
    //////// базовые параметры енд

    //// параметры баланса
    //максимальное отклонеие относительно базового значения ((1-+k)*base)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float SpeedScale{};
    //максимальное отклонеие относительно базового значения ((1-+k)*base)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float AccelerationScale{};
    ///максимальное отклонеие относительно базового значения ((1-+k)*base)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float AnimPlayRateScale{};
    ///максимальное отклонеие относительно базового значения ((1-+k)*base)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float FrictionScale{};

    // параметры формы кривой, чем больше мод тем меньше зименяется функция от аргумента
    //  при больших значениях базового значения делать меньше mod
    // форма кривой для расчета влияния от параметра
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "0.0", UIMin = "0.0"))
    float abilityMod{};
    // форма кривой для расчета влияния от скилла
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "0.0", UIMin = "0.0"))
    float skillMod{};
    // форма кривой для расчета влияния от нагруженности
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "0.0", UIMin = "0.0"))
    float weightMod{};

    // просто коэффициент влияния веса на макс. скорость
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Params",
              meta = (ClampMin = "0.0", UIMin = "0.0"))
    float weightFactorBalanceMod{};
    //////////////////======= рпг параметры энд =========///////
    // тру если персонваж был в присяде перед тем как начать падать
    bool bWasCrouching{ false };
    bool bWasJumpingOff{ false }; // тру если персонаж прыгал от стены до полета
    bool bWasJustJumping{ false };
    bool bWantToSprint{ false }; //тру если пытается перейти на спринт
    bool bIsRunning{ true }; // тру если персонаж в режима бега
    bool bIsSprinting{ false }; // тру если персонаж в режиме спринта
    // таймер по которому тратится стамина при спринте
    FTimerHandle SprintConsumeTimer;
    float SprintConsumeTime;
    void OnSprintConsume();
    virtual void Update();
    // false если перс перегружен или замедлен до остановки
    bool CanMove() const;
    float GetWeightFactor() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
    FMoveStaminaCosts StaminaCostValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
              meta     = (Bitmask, BitmaskEnum = "EMoveAbilities"),
              Category = "Character Movement")
    int MoveAbilitiesDefault;
    int MoveAbilitiesCurrent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionMoveSpeed{};
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionRotationSpeed{};

    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void SetUpdate() { IsDirty = true; }
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void UnSetUpdate() { IsDirty = false; }
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    bool IsUpdateNeeded() { return IsDirty; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
    bool ShowDebugInfo;

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
    ///
    float JumpWallZVelocity;
    float JumpWallXYVelocity;
    float MaxOverweightMoveFactor;
    float AnimPlayRate;
    float WeightFactor;

    //========================== Animation Related =========================//

    //Interrupt and AninEnd Actions (UFUNCTION) for delegate binding
    // Note: no need to unbind function from delegate when done

    // todo make interrupts for all movement modes
public:
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnParkourEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnDodgeEnded(UAnimMontage *animMontage, bool bInterrupted);

    ////////////////// ======================== /////////////////////

    ///////////=========== smoooth transition ================///////
public:
    void TransitionStart(const FVector &EndLoc,
                         const FRotator &EndRot,
                         UAnimMontage *AnimToPlay,
                         AnimTypes Type,
                         ECustomMovementMode InMode);
    // todo
    void TransitionStart(const UCurveFloat &Curve,
                         UAnimMontage *AnimToPlay,
                         AnimTypes Type,
                         ECustomMovementMode InMode);
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TransitionInterrupt();
    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsTransitionInProgress() { return bIsTransitioning; }

protected:
    void TransitionCleanUp();

    TUniquePtr<FVector> TransitionLocEnd  = nullptr;
    TUniquePtr<FRotator> TransitionRotEnd = nullptr;
    UAnimMontage *CurrentTransitionAnim   = nullptr;
    EMovementMode TransitionFinishMode;
    // is currently moving
    bool bIsTransitioning{ false };
    AnimTypes animType;

    void TransitionProcess(float deltaTime);
    void On_TransitionFinished();
    ///////////////////// ================= /////////////////
    TMap<AnimTypes, void (UBaseMovementComponent::*)(UAnimMontage *, bool)>
      AnimDelegates;

    bool IsDirty = false;
    class ACharacterPlayer *Character;
};
