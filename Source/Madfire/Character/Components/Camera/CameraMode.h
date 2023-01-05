#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveVector.h"

#include "CameraMode.generated.h"


UENUM(BlueprintType)
enum class ECameraModeBlendFunction : uint8
{
    // Does a simple linear interpolation.
    Linear,

    // Immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by the exponent.
    EaseIn,

    // Smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by the exponent.
    EaseOut,

    // Smoothly accelerates and decelerates.  Ease amount controlled by the exponent.
    EaseInOut
};


struct FCameraModeView
{
    FCameraModeView();

    void Blend(const FCameraModeView& Other, float OtherWeight);

    FVector Location;
    FRotator Rotation;
    FRotator ControlRotation;
    float FieldOfView;
};

/**
 * Struct defining a Whisker ray used for camera penetration avoidance.
 */
USTRUCT()
struct FPenetrationAvoidanceWhisker
{
    GENERATED_BODY()

    /** FRotator describing deviance from main ray */
    UPROPERTY(EditAnywhere, Category = PenetrationAvoidanceWhisker)
    FRotator AdjustmentRot;

    /** how much this Whisker affects the final position if it hits the world */
    UPROPERTY(EditAnywhere, Category = PenetrationAvoidanceWhisker)
    float WorldWeight;

    /** how much this Whisker affects the final position if it hits a APawn (setting to 0 will not attempt to collide with pawns at all) */
    UPROPERTY(EditAnywhere, Category = PenetrationAvoidanceWhisker)
    float PawnWeight;

    /** extent to use for collision when tracing this Whisker */
    UPROPERTY(EditAnywhere, Category = PenetrationAvoidanceWhisker)
    float Extent;

    /** minimum frame interval between traces with this Whisker if nothing was hit last frame */
    UPROPERTY(EditAnywhere, Category = PenetrationAvoidanceWhisker)
    int32 TraceInterval;

    /** number of frames since this Whisker was used */
    UPROPERTY()
    int32 FramesUntilNextTrace;


    FPenetrationAvoidanceWhisker()
      : AdjustmentRot(ForceInit), WorldWeight(0), PawnWeight(0), Extent(0),
        TraceInterval(0), FramesUntilNextTrace(0)
    {}

    FPenetrationAvoidanceWhisker(const FRotator &InAdjustmentRot,
                                    const float &InWorldWeight,
                                    const float &InPawnWeight,
                                    const float &InExtent,
                                    const int32 &InTraceInterval        = 0,
                                    const int32 &InFramesUntilNextTrace = 0)
      : AdjustmentRot(InAdjustmentRot), WorldWeight(InWorldWeight),
        PawnWeight(InPawnWeight), Extent(InExtent), TraceInterval(InTraceInterval),
        FramesUntilNextTrace(InFramesUntilNextTrace)
    {}
};


class UCameraMode;
UCLASS()
class UCameraModeBlender : public UObject
{
    GENERATED_BODY()

public:
    void PushCameraMode(TSubclassOf<UCameraMode> ActiveCameraMode);
    bool UpdateBlend(float DeltaTime, FCameraModeView& OutCameraView);
    // can return null
    UCameraMode *GetActiveCamera() const;

private:
    UCameraMode* GetCameraModeInstance(TSubclassOf<UCameraMode> CameraModeClass);

    UPROPERTY()
    TArray<UCameraMode*> CameraModeStack;
    UPROPERTY()
    TArray<UCameraMode*> CameraModeInstances;
};

/*
* Borrowed mostly from Epic's Lyra project
*/
UCLASS(Abstract, Blueprintable)
class MADFIRE_API UCameraMode : public UObject
{
    GENERATED_BODY()

public:
    UCameraMode();

    AActor* GetTargetActor() const;
    UWorld* GetWorld() const override;

    void Initialize();
    const FCameraModeView& GetCameraModeView() const { return View; }
    void UpdateCameraMode(float DeltaTime);
    float GetBlendTime() const { return BlendTime; }
    float GetBlendWeight() const { return BlendWeight; }
    void SetBlendWeight(float Weight);

    void AdjustCameraZoom(float Offset);

protected:
    FVector GetPivotLocation() const;
    FRotator GetPivotRotation() const;
    void UpdateCrouchOffset(float DeltaTime);
    void UpdateZoomOffset(float DeltaTime);

    void UpdatePreventPenetration(float DeltaTime);
    void PreventCameraPenetration(class AActor const &ViewTarget, FVector const &SafeLoc,
                                  FVector &CameraLoc, float const &DeltaTime,
                                  float &DistBlockedPct, bool bSingleRayOnly);

    // View output produced by the camera mode.
    FCameraModeView View;

    // The horizontal field of view (in degrees).
    UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0"))
    float FieldOfView;

    // Minimum view pitch (in degrees).
    UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
    float ViewPitchMin;

    // Maximum view pitch (in degrees).
    UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
    float ViewPitchMax;

    // How long it takes to blend in this mode.
    UPROPERTY(EditDefaultsOnly, Category = "Blending")
    float BlendTime;

    // Function used for blending.
    UPROPERTY(EditDefaultsOnly, Category = "Blending")
    ECameraModeBlendFunction BlendFunction;

    // Exponent used by blend functions to control the shape of the curve.
    UPROPERTY(EditDefaultsOnly, Category = "Blending")
    float BlendExponent;

    // TODO use RuntimeFloatCurves after UE5 update for inline editing of offsets
    // Curve that defines local-space offsets from the target using the view pitch to evaluate the curve.
    UPROPERTY(EditDefaultsOnly, Category = "Third Person")
    UCurveVector* TargetOffsetCurve;
    // Alters the speed that a crouch offset is blended in or out
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
    float CrouchOffsetBlendMultiplier = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
    bool AllowZoom = true;
    // zoom speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
    float ZoomBlendMultiplier = 5.0f;
    // max zoom ratio from default
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person", Meta = (UIMin = "1.0", ClampMin = "1.0"))
    float CameraDistanceMaxMod = 1.5f;
    // max zoom distance from camera attachment pos towards the player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person",
              Meta = (UIMin = "0.0", ClampMin = "0.0"))
    float CameraDistanceMin;

    FVector InitialZoomOffset;
    FVector CurrentZoomOffset;
    FVector TargetZoomOffset;
    float CameraZoomBlendPct = 1.0f;
    float CameraDistanceDefault;
    float CameraDistanceMax;

    // Linear blend alpha used to determine the blend weight.
    float BlendAlpha;

    // Blend weight calculated using the blend alpha and function.
    float BlendWeight;

    FVector InitialCrouchOffset = FVector::ZeroVector;
    FVector TargetCrouchOffset  = FVector::ZeroVector;
    float CrouchOffsetBlendPct  = 1.0f;
    FVector CurrentCrouchOffset = FVector::ZeroVector;

    // Penetration prevention
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float PenetrationBlendInTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float PenetrationBlendOutTime = 0.15f;

    /** If true, does collision checks to keep the camera out of the world. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool PreventPenetration = true;

    /** If true, try to detect nearby walls and move the camera in anticipation.  Helps prevent popping. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool DoPredictiveAvoidance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionPushOutDistance = 2.f;

    /** When the camera's distance is pushed into this percentage of its full distance due to penetration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float ReportPenetrationPercent = 0.f;

    /**
     * These are the Whisker rays that are used to find where to place the camera.
     * Index: 0  : This is the normal Whisker we use to prevent collisions.
     * Index: 1+ : These Whiskers are used if you bDoPredictiveAvoidance=true, to scan for potential impacts if the player
     *             were to rotate towards that direction and primitively collide the camera so that it pulls in before
     *             impacting the occluder.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Collision")
    TArray<FPenetrationAvoidanceWhisker> PenetrationAvoidanceWhiskers;

    float AimLineToDesiredPosBlockedPct;
};
