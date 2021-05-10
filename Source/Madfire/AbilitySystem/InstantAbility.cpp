#include "InstantAbility.h"
#include "Character/Components/AbilitySystemComponent.h"
#include "Character/CharacterBase.h"

void UInstantAbility::AbilityInit()
{
    CurrentCharges = MaxCharges;
    OnPrecastFinish.BindUObject(this,&UInstantAbility::PrecastFinish);
}

void UInstantAbility::AbilitySetup(UAbilitySystemComponent *Component)
{
    Super::AbilitySetup(Component);
}

void UInstantAbility::PrecastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(!bInterrupted)
    {
        CurrentCharges--;
        CastNotify();
    }
    else
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Instant Ability: Interrupted"));
        Interrupt();
    }

    StartCooldown();
}

void UInstantAbility::Activate()
{
    StopCooldown();

    if(!StartAnimation(PrecastSectionName,OnPrecastFinish))
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Instant Ability: OnActivate No Cast Montage"));
    }
    else
    {}

    Super::Activate();
}

void UInstantAbility::Release()
{
    Super::Release();
}

void UInstantAbility::Cancel()
{
    Super::Cancel();
    StopAnimation();
}

void UInstantAbility::Interrupt()
{
    Super::Interrupt();
}

void UInstantAbility::CastNotify()
{
    OnCastNotify();
}

