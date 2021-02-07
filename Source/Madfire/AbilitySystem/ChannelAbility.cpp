#include "ChannelAbility.h"

#include "AbilitySystemLog.h"

void UChannelAbility::OnEventKeyPressed_Implementation() { StartCast(); }

void UChannelAbility::OnEventKeyReleased_Implementation() { StopCast(); }

void UChannelAbility::InterruptSkill_Implementation()
{
    //still needs to implement;
}

void UChannelAbility::StopSkill_Implementation()
{
    //still needs to implement;
}

void UChannelAbility::StartCast(void)
{
    CastState = 0;

    if (!isValid())
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Skill: skill is not valid"));
        return;
    }

    if (CheckRequirements_Implementation())
    {
        ConsumeRequirements();
        CastState = 1;
        isCasting = true;
        Caster->GetWorld()->GetTimerManager().SetTimer(
          CastTimer, this, &UChannelAbility::OnSkillNotify, CastTime, true);
    }
}

void UChannelAbility::StopCast(void)
{
    isCasting = false;
    Caster->GetWorld()->GetTimerManager().ClearTimer(ChannelTimer);
    //GetWorldTimerManager().ClearTimer(ChannelTimer);
    onCooldown = true;
}

void UChannelAbility::OnSkillNotify(void)
{
    CastState = 2;
    Caster->GetWorld()->GetTimerManager().ClearTimer(CastTimer);
    //GetWorldTimerManager().ClearTimer(CastTimer);
    Caster->GetWorld()->GetTimerManager().SetTimer(
      ChannelTimer, this, &UChannelAbility::OnTick, TickTime, true);
}

void UChannelAbility::OnTick(void)
{
    if (!isCasting || !CheckRequirements_Implementation() || (CurrentCharges <= 0))
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("no charges"));
        StopCast();
    }
    else
    {
        for (auto &component : SkillComponents)
        {
            auto Obj = UAbilityManager::SpawnComponent(Caster, component, 0);
        }
        --CurrentCharges;
    }
}
