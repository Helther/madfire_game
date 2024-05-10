#include "InstantAbility.h"
#include "Character/Components/CombatComponent.h"
#include "Character/CharacterBase.h"

void UInstantAbility::AbilityInit()
{
    CurrentCharges = MaxCharges;
    OnPrecastFinish.BindUObject(this,&UInstantAbility::PrecastFinish);
}

void UInstantAbility::AbilitySetup(ACharacter *Component)
{
    Super::AbilitySetup(Component);
}

void UInstantAbility::PrecastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(!bInterrupted)
    {
        CurrentCharges--;
        CastNotify();
        bIsActive = false;
        //check(CombatComponent && "Cancel: Bad component!");
        //CombatComponent->UpdateCombatFSM();
    }
    else
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Instant Ability: Interrupted"));
        Interrupt();
    }

    StartCooldown();
}

void UInstantAbility::Activate_deprecated()
{
    StopCooldown();

    if(!StartAnimation(PrecastSectionName,OnPrecastFinish))
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Instant Ability: OnActivate No Cast Montage"));
    }
    else
    {}
    bIsActive = true;
    Super::Activate_deprecated();
}

void UInstantAbility::Release_deprecated()
{
    Super::Release_deprecated();
}

void UInstantAbility::Cancel_deprecated()
{
    Super::Cancel_deprecated();
    StopAnimation();
}

void UInstantAbility::Interrupt_deprecated()
{
    Super::Interrupt_deprecated();
}

void UInstantAbility::CastNotify()
{
    OnCastNotify();
}
