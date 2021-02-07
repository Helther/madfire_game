// Fill out your copyright notice in the Description page of Project Settings.

#include "StandaloneAbility.h"
void UStandaloneAbility::OnEventKeyPressed_Implementation() { StartCast(); }

void UStandaloneAbility::OnEventKeyReleased_Implementation()
{
    if (isImpowerable && isCasting)
    {
        isCasting = false;
    }
}

void UStandaloneAbility::InterruptSkill_Implementation()
{
    //still needs to implement;
}

void UStandaloneAbility::StopSkill_Implementation()
{
    //still needs to implement;
}

void UStandaloneAbility::StartCast(void)
{
    CastState = 0;
    if (!isValid())
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Skill: skill is not valid"));
        return;
    }

    if (CheckRequirements_Implementation() && (CurrentCharges > 0))
    {
        ConsumeRequirements();
        CastState = 1;
        isCasting = true;
        if (isImpowerable)
        {
            currentPressureTime = 0.0f;
            impower             = 0;
            Caster->GetWorld()->GetTimerManager().SetTimer(
              CastTimer, this, &UStandaloneAbility::OnTick, 0.25, true);
        }
        else
        {
            Caster->GetWorld()->GetTimerManager().SetTimer(
              CastTimer, this, &UStandaloneAbility::OnSkillNotify, CastTime, true);
        }
    }
}

void UStandaloneAbility::OnSkillNotify(void)
{
    CastState = 2;
    Caster->GetWorld()->GetTimerManager().ClearTimer(CastTimer);
    for (auto &component : SkillComponents)
    {
        auto Obj = UAbilityManager::SpawnComponent(Caster, component, impower);
    }

    StopCast();

    UE_LOG(AbilitySystemLog, Error, TEXT("percentage: %d"), impower);
}

void UStandaloneAbility::OnTick(void)
{
    if ((currentPressureTime < MaximumPressureTime) && isCasting)
    {
        currentPressureTime += 0.25f;
    }
    else
    {
        Caster->GetWorld()->GetTimerManager().ClearTimer(CastTimer);
        float buffer = currentPressureTime / MaximumPressureTime;
        impower      = buffer * 100;
        OnSkillNotify();
    }
}

void UStandaloneAbility::StopCast(void)
{
    isCasting  = false;
    onCooldown = true;
}