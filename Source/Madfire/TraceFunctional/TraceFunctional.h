#pragma once

#include "Classes/Engine/World.h"
#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "TraceDefines.h"

#include "TraceFunctional.generated.h"


/*
* Collision Params
*
* bFindInitialOverlaps - Whether we want to find out initial overlap or not.
* bIgnoreBlocks - Whether to ignore blocking results.
* bIgnoreTouches - Whether to ignore touch/overlap results.
* bReturnPhysicalMaterial - Only fill in the PhysMaterial field of
* bTraceAsyncScene - Whether we should perform the trace in the asynchronous scene. Default is false.
* bTraceComplex - Whether we should trace against complex collision
* OwnerTag - Tag used to indicate an owner for this trace
* TraceTag - Tag used to provide extra information or filtering for debugging of the trace (e.g. Collision Analyzer)
*/


//component must calculate trace start, end and rotation
USTRUCT(BlueprintType)
struct FTraceParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor *> ActorsToIgnore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool isDebug = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool TraceComplex = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool TraceAsyncScene = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IgnoreBlocks = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IgnoreTouches = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName OwnerTag = "NullOwnerTag";

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TraceTag = "NullTraceTag";

    explicit inline operator FCollisionQueryParams() const noexcept
    {
        FCollisionQueryParams params;
        params.AddIgnoredActors(ActorsToIgnore);
        params.bTraceComplex = TraceComplex;
        //params.bTraceAsyncScene = TraceAsyncScene; //changed in new version
        params.bIgnoreBlocks  = IgnoreBlocks;
        params.bIgnoreTouches = IgnoreTouches;
        params.OwnerTag       = OwnerTag;
        params.TraceTag       = TraceTag;
        return params;
    }
};


USTRUCT(BlueprintType)
struct FTraceSpawnParams
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector Start;

    UPROPERTY(BlueprintReadWrite)
    FVector End;

    UPROPERTY(BlueprintReadWrite)
    FQuat Rotation;

    //warning this property does not exists for line traces
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCollisionResponseContainer CollisionResponse;
};


UCLASS()
class UTraceManager : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    ///***COMPLETED***///
    //Line Traces
    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool   LineTraceSingleByChannel(UWorld *World,
                               const FTraceParams &TraceParams,
                               const FTraceSpawnParams &TraceSpawnParams,
                               FHitResult &HitOut,
                               ECollisionChannel CollisionChannel = ECC_Pawn,
                               bool ReturnPhysMat                 = false)
    {

        if (!World)
        {
            return false;
        }

        FCollisionQueryParams Params(TraceParams);
        HitOut = FHitResult(ForceInit);
        FCollisionResponseParams ResponseParams;
        ResponseParams.CollisionResponse = TraceSpawnParams.CollisionResponse;

        if (TraceParams.isDebug)
        {
            //World->DebugDrawTraceTag = Params.TraceTag;
        }

        bool isHit = World->LineTraceSingleByChannel(
          HitOut, TraceSpawnParams.Start, TraceSpawnParams.End, CollisionChannel, Params);

        if (TraceParams.isDebug && isHit)
        {
            FColor color;
            DrawDebugLine(World, HitOut.TraceStart, HitOut.ImpactPoint,
                          color.MakeRandomColor(), false, 10, 0, 1);
        }

        return isHit;
    }

    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE UWorld *GetWorldOf(AActor *Actor) { return Actor->GetWorld(); }


    ///TODO: MAKE ROTATION OF THIS JOHN
    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool TraceSpawnParamsEvaluation(
      ACharacter *Character, FVector &Start, FVector &End, FQuat &Rotation,
      FRotator RotationOffset = FRotator(0, 0, 0),
      FVector LocationOffset = FVector(0, 0, 0), float Length = 1.0f)
    {
        if (!Character)
        {
            return false;
        }

        FVector ActorRotation = Character->GetControlRotation().Vector();
        Rotation.X            = ActorRotation.X;
        Rotation.Y            = ActorRotation.Y;
        Rotation.Z            = ActorRotation.Z;
        Rotation.W            = 1.0f;

        Start = Character->GetActorLocation() + LocationOffset;
        End   = ActorRotation * Length + Start;

        return true;
    }

    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool
      LineTraceMultiByChannel(UWorld *World,
                              const FTraceParams &TraceParams,
                              const FTraceSpawnParams &TraceSpawnParams,
                              TArray<struct FHitResult> &HitOut,
                              ECollisionChannel CollisionChannel = ECC_Pawn,
                              bool ReturnPhysMat                 = false)
    {

        if (!World)
        {
            return false;
        }

        FCollisionQueryParams Params(TraceParams);

        if (TraceParams.isDebug)
        {
            //World->DebugDrawTraceTag = Params.TraceTag;
        }

        bool isHit = World->LineTraceMultiByChannel(
          HitOut, TraceSpawnParams.Start, TraceSpawnParams.End, CollisionChannel, Params);

        if (TraceParams.isDebug && isHit)
        {
            FColor color;
            for (auto &hits : HitOut)
            {
                DrawDebugLine(World, hits.TraceStart, hits.ImpactPoint,
                              color.MakeRandomColor(), false, 10, 0, 1);
            }
        }

        return isHit;
    }

    //Sphere Traces
    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool
      SphereTraceSingleByChannel(UWorld *World,
                                 const float Radius,
                                 const FTraceParams &TraceParams,
                                 const FTraceSpawnParams &TraceSpawnParams,
                                 FHitResult &HitOut,
                                 ECollisionChannel CollisionChannel = ECC_Pawn,
                                 bool ReturnPhysMat                 = false)
    {
        if (!World)
        {
            return false;
        }

        FCollisionQueryParams Params(TraceParams);
        HitOut = FHitResult(ForceInit);
        FCollisionResponseParams ResponseParams;
        ResponseParams.CollisionResponse = TraceSpawnParams.CollisionResponse;

        if (TraceParams.isDebug)
        {
            //World->DebugDrawTraceTag = Params.TraceTag;
        }

        bool isHit = World->SweepSingleByChannel(
          HitOut, TraceSpawnParams.Start, TraceSpawnParams.End, TraceSpawnParams.Rotation,
          CollisionChannel, FCollisionShape::MakeSphere(Radius), Params, ResponseParams);

        if (TraceParams.isDebug && isHit)
        {
            FColor color;
            DrawDebugLine(World, HitOut.TraceStart, HitOut.ImpactPoint,
                          color.MakeRandomColor(), false, 10, 0, 1);
        }

        return isHit;
    }


    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool
      SphereTraceMultiByChannel(UWorld *World,
                                const float Radius,
                                const FTraceParams &TraceParams,
                                const FTraceSpawnParams &TraceSpawnParams,
                                TArray<struct FHitResult> &OutHits,
                                ECollisionChannel CollisionChannel = ECC_Pawn,
                                bool ReturnPhysMat                 = false)
    {

        if (!World)
        {
            return false;
        }

        FCollisionQueryParams Params(TraceParams);

        FCollisionResponseParams ResponseParams;
        ResponseParams.CollisionResponse = TraceSpawnParams.CollisionResponse;

        if (TraceParams.isDebug)
        {
            //World->DebugDrawTraceTag = Params.TraceTag;
        }

        bool isHit = World->SweepMultiByChannel(
          OutHits, TraceSpawnParams.Start, TraceSpawnParams.End,
          TraceSpawnParams.Rotation, CollisionChannel,
          FCollisionShape::MakeSphere(Radius), Params, ResponseParams);

        if (TraceParams.isDebug && isHit)
        {
            FColor color;
            for (auto &hits : OutHits)
            {
                DrawDebugLine(World, hits.TraceStart, hits.ImpactPoint,
                              color.MakeRandomColor(), false, 10, 0, 1);
            }
        }

        return isHit;
    }

    //Box Trace
    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool
      BoxTraceSingleByChannel(UWorld *World,
                              const FVector &BoxHalfExtent,
                              const FTraceParams &TraceParams,
                              const FTraceSpawnParams &TraceSpawnParams,
                              FHitResult &HitOut,
                              ECollisionChannel CollisionChannel = ECC_Pawn,
                              bool ReturnPhysMat                 = false)
    {

        if (!World)
        {
            return false;
        }

        FCollisionQueryParams Params(TraceParams);
        HitOut = FHitResult(ForceInit);
        FCollisionResponseParams ResponseParams;
        ResponseParams.CollisionResponse = TraceSpawnParams.CollisionResponse;

        if (TraceParams.isDebug)
        {
            //World->DebugDrawTraceTag = Params.TraceTag;
        }

        auto Rotation = TraceSpawnParams.Rotation;

        if (!Rotation.IsNormalized())
        {
            Rotation.Normalize();
        }

        bool isHit = World->SweepSingleByChannel(
          HitOut, TraceSpawnParams.Start, TraceSpawnParams.End, Rotation,
          CollisionChannel, FCollisionShape::MakeBox(BoxHalfExtent), Params,
          ResponseParams);

        if (TraceParams.isDebug && isHit)
        {
            FColor color;
            DrawDebugLine(World, HitOut.TraceStart, HitOut.ImpactPoint,
                          color.MakeRandomColor(), false, 10, 0, 1);
        }

        return isHit;
    }

    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool
      BoxTraceMultiByChannel(UWorld *World,
                             const FVector &BoxHalfExtent,
                             const FTraceParams &TraceParams,
                             const FTraceSpawnParams &TraceSpawnParams,
                             TArray<FHitResult> &OutHits,
                             ECollisionChannel CollisionChannel = ECC_Pawn,
                             bool ReturnPhysMat                 = false)
    {

        if (!World)
        {
            return false;
        }

        FCollisionQueryParams Params(TraceParams);

        FCollisionResponseParams ResponseParams;
        ResponseParams.CollisionResponse = TraceSpawnParams.CollisionResponse;

        if (TraceParams.isDebug)
        {
            //World->DebugDrawTraceTag = Params.TraceTag;
        }

        auto Rotation = TraceSpawnParams.Rotation;

        if (!Rotation.IsNormalized())
        {
            Rotation.Normalize();
        }

        bool isHit = World->SweepMultiByChannel(
          OutHits, TraceSpawnParams.Start, TraceSpawnParams.End, Rotation,
          CollisionChannel, FCollisionShape::MakeBox(BoxHalfExtent), Params,
          ResponseParams);

        if (TraceParams.isDebug && isHit)
        {
            FColor color;
            for (auto &hits : OutHits)
            {
                DrawDebugLine(World, hits.TraceStart, hits.ImpactPoint,
                              color.MakeRandomColor(), false, 10, 0, 1);
            }
        }

        return isHit;
    }

    //Capsule Trace
    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool
      CapsuleTraceSingleByChannel(UWorld *World,
                                  float CapsuleRadius,
                                  float CapsuleHalfHeight,
                                  const FTraceParams &TraceParams,
                                  const FTraceSpawnParams &TraceSpawnParams,
                                  FHitResult &HitOut,
                                  ECollisionChannel CollisionChannel = ECC_Pawn,
                                  bool ReturnPhysMat                 = false)
    {

        if (!World)
        {
            return false;
        }

        FCollisionQueryParams Params(TraceParams);
        HitOut = FHitResult(ForceInit);
        FCollisionResponseParams ResponseParams;
        ResponseParams.CollisionResponse = TraceSpawnParams.CollisionResponse;

        if (TraceParams.isDebug)
        {
            //World->DebugDrawTraceTag = Params.TraceTag;
        }

        auto Rotation = TraceSpawnParams.Rotation;

        if (!Rotation.IsNormalized())
        {
            Rotation.Normalize();
        }

        bool isHit = World->SweepSingleByChannel(
          HitOut, TraceSpawnParams.Start, TraceSpawnParams.End, Rotation,
          CollisionChannel,
          FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), Params,
          ResponseParams);

        if (TraceParams.isDebug && isHit)
        {
            FColor color;
            DrawDebugLine(World, HitOut.TraceStart, HitOut.ImpactPoint,
                          color.MakeRandomColor(), false, 10, 0, 1);
        }

        return isHit;
    }

    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool
      CapsuleTraceMultiByChannel(UWorld *World,
                                 float CapsuleRadius,
                                 float CapsuleHalfHeight,
                                 const FTraceParams &TraceParams,
                                 const FTraceSpawnParams &TraceSpawnParams,
                                 TArray<struct FHitResult> &OutHits,
                                 ECollisionChannel CollisionChannel = ECC_Pawn,
                                 bool ReturnPhysMat                 = false)
    {

        if (!World)
        {
            return false;
        }

        FCollisionQueryParams Params(TraceParams);

        FCollisionResponseParams ResponseParams;
        ResponseParams.CollisionResponse = TraceSpawnParams.CollisionResponse;

        if (TraceParams.isDebug)
        {
            //World->DebugDrawTraceTag = Params.TraceTag;
        }

        auto Rotation = TraceSpawnParams.Rotation;

        if (!Rotation.IsNormalized())
        {
            Rotation.Normalize();
        }

        bool isHit = World->SweepMultiByChannel(
          OutHits, TraceSpawnParams.Start, TraceSpawnParams.End, Rotation,
          CollisionChannel,
          FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), Params,
          ResponseParams);

        if (TraceParams.isDebug && isHit)
        {
            FColor color;
            for (auto &hits : OutHits)
            {
                DrawDebugLine(World, hits.TraceStart, hits.ImpactPoint,
                              color.MakeRandomColor(), false, 10, 0, 1);
            }
        }

        return isHit;
    }
    ///***COMPLETED***///
};
