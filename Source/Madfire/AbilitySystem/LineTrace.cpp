// Fill out your copyright notice in the Description page of Project Settings.

#include "LineTrace.h"
#include "AbilitySystemLog.h"


ALineTrace::ALineTrace()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ALineTrace::BeginPlay()
{
    Super::BeginPlay();

}

void ALineTrace::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

bool ALineTrace::ComponentSetup_Implementation(ACharacter *Parent, int32 ActionParameter)
{
	check(Parent != nullptr && Parent->GetWorld() != nullptr && "Component setup: parent or world is invalid");

    if (!IsSelfApply)
    {
		TraceParams.ActorsToIgnore.Add(Parent);
    }

    SpawnParams.Start = Parent->GetActorLocation();
    SpawnParams.End = Parent->GetActorForwardVector() * 1000;

    SpawnParams.End.Z = 0.0f;
    SpawnParams.End += SpawnParams.Start;

    if (!IsMultipleTrace)
    {
        FHitResult HitResult;
        UTraceManager::LineTraceSingleByChannel(Parent->GetWorld(), TraceParams, SpawnParams, HitResult, CollisionChannel, ReturnPhysMat);
        if (HitResult.GetActor())
        {
            AAbilityComponent::Execute(HitResult.GetActor(), ActionList, ActionEmpowerment, Caster, this);
        }
    }
    else
    {
        TArray<struct FHitResult> HitResults;
        UTraceManager::LineTraceMultiByChannel(Parent->GetWorld(), TraceParams, SpawnParams, HitResults, CollisionChannel, ReturnPhysMat);
        for (auto & target : HitResults)
        {
            if (target.GetActor())
            {
                    AAbilityComponent::Execute(target.GetActor(), ActionList, ActionEmpowerment, Caster, this);
            }
        }
    }

    return true;
}

