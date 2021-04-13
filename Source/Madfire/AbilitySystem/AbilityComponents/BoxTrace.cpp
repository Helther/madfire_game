// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxTrace.h"
#include "AbilitySystem/AbilitySystemLog.h"
/*

ABoxTrace::ABoxTrace() { PrimaryActorTick.bCanEverTick = true; }

void ABoxTrace::BeginPlay() { Super::BeginPlay(); }

void ABoxTrace::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

bool ABoxTrace::ComponentSetup_Implementation(ACharacter *Parent, int32 ActionParameter)
{
    check(Parent != nullptr && Parent->GetWorld() != nullptr
          && "Component setup: parent or world is invalid");

    if (!IsSelfApply)
    {
        TraceParams.ActorsToIgnore.Add(Parent);
    }

    FVector ActorRotation  = Parent->GetActorRotation().Vector();
    SpawnParams.Rotation.X = ActorRotation.X;
    SpawnParams.Rotation.Y = ActorRotation.Y;
    SpawnParams.Rotation.Z = ActorRotation.Z;
    SpawnParams.Rotation.W = 1.0f;

    SpawnParams.Start = Parent->GetActorLocation();
    SpawnParams.End   = Parent->GetActorForwardVector() * 1000;

    SpawnParams.End.Z = 0.0f;
    SpawnParams.End += SpawnParams.Start;

    if (!SpawnParams.Rotation.IsNormalized())
    {
        SpawnParams.Rotation.Normalize();
    }

    if (!IsMultipleTrace)
    {
        FHitResult HitResult;
        UTraceManager::BoxTraceSingleByChannel(Parent->GetWorld(), BoxHalfExtent,
                                               TraceParams, SpawnParams, HitResult,
                                               CollisionChannel, ReturnPhysMat);
        if (HitResult.GetActor())
        {
            AAbilityComponent::Execute(HitResult.GetActor(), ActionList,
                                       ActionEmpowerment, Caster, this);
        }
    }
    else
    {
        TArray<struct FHitResult> HitResults;
        UTraceManager::BoxTraceMultiByChannel(Parent->GetWorld(), BoxHalfExtent,
                                              TraceParams, SpawnParams, HitResults,
                                              CollisionChannel, ReturnPhysMat);
        for (auto &target : HitResults)
        {
            if (target.GetActor())
            {
                AAbilityComponent::Execute(target.GetActor(), ActionList,
                                           ActionEmpowerment, Caster, this);
            }
        }
    }

    return true;
}
*/
