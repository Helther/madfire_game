// Fill out your copyright notice in the Description page of Project Settings.

#include "CapsuleTrace.h"
#include "AbilitySystemLog.h"


ACapsuleTrace::ACapsuleTrace()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACapsuleTrace::BeginPlay()
{
	Super::BeginPlay();

}

void ACapsuleTrace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ACapsuleTrace::ComponentSetup_Implementation(ACharacter *Parent, int32 ActionParameter)
{
	check(Parent != nullptr && Parent->GetWorld() != nullptr && "Component setup: parent or world is invalid");

	if (!IsSelfApply)
	{
		TraceParams.ActorsToIgnore.Add(Parent);
	}

	FVector ActorRotation = Parent->GetActorRotation().Vector();
	SpawnParams.Rotation.X = ActorRotation.X;
	SpawnParams.Rotation.Y = ActorRotation.Y;
	SpawnParams.Rotation.Z = ActorRotation.Z;
	SpawnParams.Rotation.W = 1.0f;

	SpawnParams.Start = Parent->GetActorLocation();
	SpawnParams.End = Parent->GetActorForwardVector() * 1000;

	SpawnParams.End.Z = 0.0f;
	SpawnParams.End += SpawnParams.Start;
	
	if (!SpawnParams.Rotation.IsNormalized())
	{
		SpawnParams.Rotation.Normalize();
	}

	if (!IsMultipleTrace)
	{
		FHitResult HitResult;
		UTraceManager::CapsuleTraceSingleByChannel(Parent->GetWorld(), CapsuleRadius, CapsuleHalfHeight, TraceParams, SpawnParams, HitResult, CollisionChannel, ReturnPhysMat);
		if (HitResult.GetActor())
		{
			AAbilityComponent::Execute(HitResult.GetActor(), ActionList, ActionEmpowerment, Caster, this);
		}
	}
	else
	{
		TArray<struct FHitResult> HitResults;
		UTraceManager::CapsuleTraceMultiByChannel(Parent->GetWorld(), CapsuleRadius, CapsuleHalfHeight, TraceParams, SpawnParams, HitResults, CollisionChannel, ReturnPhysMat);
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

