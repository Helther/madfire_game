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
    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE UWorld *GetWorldOf(AActor *Actor) { return Actor->GetWorld(); }

    ///TODO: MAKE ROTATION OF THIS JOHN
    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool TraceSpawnParamsEvaluation(ACharacter *Character, 
                                                       FVector &Start, 
                                                       FVector &End, 
                                                       FQuat &Rotation,
                                                       FRotator RotationOffset = FRotator(0, 0, 0),
                                                       FVector LocationOffset = FVector(0, 0, 0), 
                                                       float Length = 1.0f)
    {
        check(Character != nullptr && "No character");

        FVector ActorRotation = Character->GetControlRotation().Vector();
        Rotation.X            = ActorRotation.X;
        Rotation.Y            = ActorRotation.Y;
        Rotation.Z            = ActorRotation.Z;
        Rotation.W            = 1.0f;

        Start = Character->GetActorLocation() + LocationOffset;
        End   = ActorRotation * Length + Start;

        return true;
    }

        ///***COMPLETED***///
    //Line Traces
    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool LineTraceSingleByChannel(UWorld *World,
                                                     const FTraceParams &TraceParams,
                                                     const FTraceSpawnParams &TraceSpawnParams,
                                                     FHitResult &HitOut,
                                                     ECollisionChannel CollisionChannel = ECC_Pawn,
                                                     bool ReturnPhysMat                 = false)
    {

        check(World != nullptr && "No world");
        FCollisionQueryParams Params(TraceParams);
        HitOut = FHitResult(ForceInit);

        bool isHit = World->LineTraceSingleByChannel(HitOut,
                                                     TraceSpawnParams.Start, 
                                                     TraceSpawnParams.End, 
                                                     CollisionChannel, 
                                                     Params);

        if (TraceParams.isDebug && isHit)
            DrawDebugLine(World, HitOut.TraceStart, HitOut.ImpactPoint, FColor::MakeRandomColor(), false, 10, 0, 1);
        
        return isHit;
    }

    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool LineTraceMultiByChannel(UWorld *World,
                                                    const FTraceParams &TraceParams,
                                                    const FTraceSpawnParams &TraceSpawnParams,
                                                    TArray<struct FHitResult> &HitOut,
                                                    ECollisionChannel CollisionChannel = ECC_Pawn,
                                                    bool ReturnPhysMat                 = false)
    {

        check(World != nullptr && "No world");
        FCollisionQueryParams Params(TraceParams);

        bool isHit = World->LineTraceMultiByChannel(HitOut, 
                                                    TraceSpawnParams.Start, 
                                                    TraceSpawnParams.End, 
                                                    CollisionChannel, 
                                                    Params);

        if (TraceParams.isDebug && isHit)
            for (auto &hits : HitOut)
                DrawDebugLine(World, hits.TraceStart, hits.ImpactPoint, FColor::MakeRandomColor(), false, 10, 0, 1);
        
        return isHit;
    }

    static bool AreaTraceSingleByChannel(UWorld *World, 
                                         FCollisionShape Shape,                                  
                                         const FTraceParams &TraceParams,
                                         const FTraceSpawnParams &TraceSpawnParams,
                                         FHitResult &HitOut,
                                         ECollisionChannel CollisionChannel = ECC_Pawn,
                                         bool ReturnPhysMat                 = false)
    {
        check(World != nullptr && "No world");
        FCollisionQueryParams Params(TraceParams);
        FCollisionResponseParams ResponseParams(TraceSpawnParams.CollisionResponse);

        bool isHit = World->SweepSingleByChannel(HitOut, 
                                                 TraceSpawnParams.Start, 
                                                 TraceSpawnParams.End, 
                                                 TraceSpawnParams.Rotation,
                                                 CollisionChannel, 
                                                 Shape, 
                                                 Params, 
                                                 ResponseParams);

        if (TraceParams.isDebug && isHit)
            DrawDebugLine(World, HitOut.TraceStart, HitOut.ImpactPoint, FColor::MakeRandomColor(), false, 10, 0, 1);
        
        return isHit;
    }

    static bool AreaTraceMultiByChannel(UWorld *World, 
                                        FCollisionShape Shape,                                  
                                        const FTraceParams &TraceParams,
                                        const FTraceSpawnParams &TraceSpawnParams,
                                        TArray<struct FHitResult> &OutHits,
                                        ECollisionChannel CollisionChannel = ECC_Pawn,
                                        bool ReturnPhysMat                 = false)
    {
        check(World != nullptr && "No world");
        FCollisionQueryParams Params(TraceParams);
        FCollisionResponseParams ResponseParams(TraceSpawnParams.CollisionResponse);

        bool isHit = World->SweepMultiByChannel(OutHits, 
                                                TraceSpawnParams.Start, 
                                                TraceSpawnParams.End, 
                                                TraceSpawnParams.Rotation,
                                                CollisionChannel, 
                                                Shape, 
                                                Params, 
                                                ResponseParams);

        if (TraceParams.isDebug && isHit)
            for (auto &hits : OutHits)
                DrawDebugLine(World, hits.TraceStart, hits.ImpactPoint, FColor::MakeRandomColor(), false, 10, 0, 1);
        
        return isHit;
    }

    //Sphere Traces
    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool SphereTraceSingleByChannel(UWorld *World,
                                                       const float Radius,
                                                       const FTraceParams &TraceParams,
                                                       const FTraceSpawnParams &TraceSpawnParams,
                                                       FHitResult &HitOut,
                                                       ECollisionChannel CollisionChannel = ECC_Pawn,
                                                       bool ReturnPhysMat                 = false)
    {
        return AreaTraceSingleByChannel(World,
                                       FCollisionShape::MakeSphere(Radius), 
                                       TraceParams,
                                       TraceSpawnParams,
                                       HitOut,
                                       CollisionChannel,
                                       ReturnPhysMat);
    }


    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool SphereTraceMultiByChannel(UWorld *World,
                                                      const float Radius,
                                                      const FTraceParams &TraceParams,
                                                      const FTraceSpawnParams &TraceSpawnParams,
                                                      TArray<struct FHitResult> &OutHits,
                                                      ECollisionChannel CollisionChannel = ECC_Pawn,
                                                      bool ReturnPhysMat                 = false)
    {
        return AreaTraceMultiByChannel(World,
                                       FCollisionShape::MakeSphere(Radius), 
                                       TraceParams,
                                       TraceSpawnParams,
                                       OutHits,
                                       CollisionChannel,
                                       ReturnPhysMat);
    }

    //Box Trace
    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool BoxTraceSingleByChannel(UWorld *World,
                                                    const FVector &BoxHalfExtent,
                                                    const FTraceParams &TraceParams,
                                                    const FTraceSpawnParams &TraceSpawnParams,
                                                    FHitResult &HitOut,
                                                    ECollisionChannel CollisionChannel = ECC_Pawn,
                                                    bool ReturnPhysMat                 = false)
    {
        return AreaTraceSingleByChannel(World,
                                       FCollisionShape::MakeBox(BoxHalfExtent), 
                                       TraceParams,
                                       TraceSpawnParams,
                                       HitOut,
                                       CollisionChannel,
                                       ReturnPhysMat);
    }

    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool BoxTraceMultiByChannel(UWorld *World,
                                                   const FVector &BoxHalfExtent,
                                                   const FTraceParams &TraceParams,
                                                   const FTraceSpawnParams &TraceSpawnParams,
                                                   TArray<FHitResult> &OutHits,
                                                   ECollisionChannel CollisionChannel = ECC_Pawn,
                                                   bool ReturnPhysMat                 = false)
    {
        return AreaTraceMultiByChannel(World,
                                       FCollisionShape::MakeBox(BoxHalfExtent), 
                                       TraceParams,
                                       TraceSpawnParams,
                                       OutHits,
                                       CollisionChannel,
                                       ReturnPhysMat);
    }

    //Capsule Trace
    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool CapsuleTraceSingleByChannel(UWorld *World,
                                                        float CapsuleRadius,
                                                        float CapsuleHalfHeight,
                                                        const FTraceParams &TraceParams,
                                                        const FTraceSpawnParams &TraceSpawnParams,
                                                        FHitResult &HitOut,
                                                        ECollisionChannel CollisionChannel = ECC_Pawn,
                                                        bool ReturnPhysMat                 = false)
    {
        return AreaTraceSingleByChannel(World,
                                       FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), 
                                       TraceParams,
                                       TraceSpawnParams,
                                       HitOut,
                                       CollisionChannel,
                                       ReturnPhysMat);
    }

    UFUNCTION(BlueprintPure, Category = "Trace")
    static FORCEINLINE bool CapsuleTraceMultiByChannel(UWorld *World,
                                                       float CapsuleRadius,
                                                       float CapsuleHalfHeight,
                                                       const FTraceParams &TraceParams,
                                                       const FTraceSpawnParams &TraceSpawnParams,
                                                       TArray<struct FHitResult> &OutHits,
                                                       ECollisionChannel CollisionChannel = ECC_Pawn,
                                                       bool ReturnPhysMat                 = false)
    {
        return AreaTraceMultiByChannel(World,
                                       FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), 
                                       TraceParams,
                                       TraceSpawnParams,
                                       OutHits,
                                       CollisionChannel,
                                       ReturnPhysMat);
    }
    ///***COMPLETED***///

    static bool AreaTraceSignleByObjectType(UWorld *World, 
                                            FCollisionShape Shape,                                  
                                            const FTraceParams &TraceParams,
                                            const FTraceSpawnParams &TraceSpawnParams,                                            
                                            FHitResult &OutHit,
                                            ECollisionChannel CollisionChannel = ECC_Pawn,
                                            bool ReturnPhysMat                 = false)
    {
        check(World != nullptr && "No world");
        FCollisionQueryParams Params(TraceParams);
        FCollisionObjectQueryParams ObjectTypes;
        ObjectTypes.AddObjectTypesToQuery(CollisionChannel);

        bool isHit = World->SweepSingleByObjectType(OutHit, 
                                                    TraceSpawnParams.Start,
                                                    TraceSpawnParams.End,
                                                    TraceSpawnParams.Rotation,
                                                    ObjectTypes,
                                                    Shape,
                                                    Params);

        if (TraceParams.isDebug && isHit)
            DrawDebugLine(World, OutHit.TraceStart, OutHit.ImpactPoint, FColor::MakeRandomColor(), false, 10, 0, 1);
        
        return isHit;
    }

    static bool AreaTraceMultiByObjectType(UWorld *World, 
                                           FCollisionShape Shape,                                  
                                           const FTraceParams &TraceParams,
                                           const FTraceSpawnParams &TraceSpawnParams,                                                   
                                           TArray<struct FHitResult> &OutHits,
                                           ECollisionChannel CollisionChannel = ECC_Pawn,
                                           bool ReturnPhysMat         = false)
    {
        check(World != nullptr && "No world");
        FCollisionQueryParams Params(TraceParams);
        FCollisionResponseParams ResponseParams(TraceSpawnParams.CollisionResponse);
        FCollisionObjectQueryParams ObjectTypes;
        ObjectTypes.AddObjectTypesToQuery(CollisionChannel);

        bool isHit = World->SweepMultiByObjectType(OutHits, 
                                                   TraceSpawnParams.Start,
                                                   TraceSpawnParams.End,
                                                   TraceSpawnParams.Rotation,
                                                   ObjectTypes,
                                                   Shape,
                                                   Params);

        if (TraceParams.isDebug && isHit)
            for (auto &hit : OutHits)
                DrawDebugLine(World, hit.TraceStart, hit.ImpactPoint, FColor::MakeRandomColor(), false, 10, 0, 1);
        
        return isHit;
    }
};
