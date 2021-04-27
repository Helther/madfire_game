#include "AbstractAbility.h"
#include "AbilitySystemLog.h"


UAbstractAbility::UAbstractAbility(const FObjectInitializer &f) :
    Super(f)
{

}



/*
bool UAbstractAbility::CheckRequirements_Implementation(void)
{
    if (Requirements.Num() == 0)
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Requirements: is not set up"));
        return true;
    }

    for (auto req : Requirements)
    {
        if (!req->CheckRequirement(Caster))
        {
            return false;
        }
    }

    return true;
}*/

void UAbstractAbility::AbilitySetup(UAbilitySystemComponent *Component)
{
    AbilitySystemComponent = Component;
    for(auto &component : AbilityComponents)
    {
        component->ComponentSetup(Component);
    }
}

void UAbstractAbility::ActivateAbility()
{
    if(CurrentCharges <= 0)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: Cooldown"));
        return;
    }
    OnActivate();
    OnAbilityStart.Broadcast(this);
}

void UAbstractAbility::RealeseAbility()
{
    OnRelease();
    OnAbilityRelease.Broadcast(this);
}

void UAbstractAbility::CancelAbility()
{
    OnCancel();
    OnAbilityCancel.Broadcast(this);
}

void UAbstractAbility::Tick(float DeltaTime)
{
    if(bIsCooldown)
    {
        if(CurrentCharges < MaxCharges)
        {
            CurrentCooldownTime+=DeltaTime;
            if(CurrentCooldownTime >= CooldownTime)
            {
                CurrentCharges+=ChargePerCooldown;
            }
        }
        else
        {
            bIsCooldown = false;
        }
    }
}

bool UAbstractAbility::IsTickable() const
{
    return true;
}

bool UAbstractAbility::IsTickableInEditor() const
{
    return false;
}

bool UAbstractAbility::IsTickableWhenPaused() const
{
    return false;
}

TStatId UAbstractAbility::GetStatId() const
{
    return TStatId();
}

float UAbstractAbility::GetCooldownTimeRemaining() const
{
    return CooldownTime - CurrentCooldownTime;
}

UWorld* UAbstractAbility::GetWorld() const
{
    return GetOuter()->GetWorld();
}
