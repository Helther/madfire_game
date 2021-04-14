// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityTargetInLine.h"
#include "AbilitySystem/AbilitySystemLog.h"


UAbilityTargetInLine::UAbilityTargetInLine()
{
    //PrimaryActorTick.bCanEverTick = true;
}
/*
void UAbilityTargetInLine::BeginPlay()
{
    Super::BeginPlay();
}

void AAbilityTargetInLine::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
*/
void UAbilityTargetInLine::Activate_Implementation()
{

}

bool UAbilityTargetInLine::ComponentSetup_Implementation(UAbilitySystemComponent *Component)
{
    Super::ComponentSetup(Component);
    /*
    check(Parent != nullptr && Parent->GetWorld() != nullptr
          && "Component setup: parent or world is invalid");

    if (!IsSelfApply)
    {
        TraceParams.ActorsToIgnore.Add(Parent);
    }

    SpawnParams.Start = Parent->GetActorLocation();
    SpawnParams.End   = Parent->GetActorForwardVector() * 1000;

    SpawnParams.End.Z = 0.0f;
    SpawnParams.End += SpawnParams.Start;

    if (!IsMultipleTrace)
    {
        FHitResult HitResult;
        UTraceManager::LineTraceSingleByChannel(Parent->GetWorld(), TraceParams,
                                                SpawnParams, HitResult, CollisionChannel,
                                                ReturnPhysMat);
        if (HitResult.GetActor())
        {
            AAbilityComponent::Execute(HitResult.GetActor(), ActionList,
                                       ActionEmpowerment, Caster, this);
        }
    }
    else
    {
        TArray<struct FHitResult> HitResults;
        UTraceManager::LineTraceMultiByChannel(Parent->GetWorld(), TraceParams,
                                               SpawnParams, HitResults, CollisionChannel,
                                               ReturnPhysMat);
        for (auto &target : HitResults)
        {
            if (target.GetActor())
            {
                AAbilityComponent::Execute(target.GetActor(), ActionList,
                                           ActionEmpowerment, Caster, this);
            }
        }
    }
    */
    return true;
}

