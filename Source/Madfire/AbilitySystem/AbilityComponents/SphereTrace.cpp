// Fill out your copyright notice in the Description page of Project Settings.

#include "SphereTrace.h"

#include "AbilitySystem/AbilitySystemLog.h"

/*
ASphereTrace::ASphereTrace() { PrimaryActorTick.bCanEverTick = true; }

void ASphereTrace::BeginPlay() { Super::BeginPlay(); }

void ASphereTrace::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

bool ASphereTrace::ComponentSetup_Implementation(ACharacter *Parent,
                                                 int32 ActionParameter)
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

    if (!IsMultipleTrace)
    {
        FHitResult HitResult;
        UTraceManager::SphereTraceSingleByChannel(Parent->GetWorld(), Radius, TraceParams,
                                                  SpawnParams, HitResult,
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
        UTraceManager::SphereTraceMultiByChannel(Parent->GetWorld(), Radius, TraceParams,
                                                 SpawnParams, HitResults,
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
