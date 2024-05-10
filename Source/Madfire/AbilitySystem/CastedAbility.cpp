#include "CastedAbility.h"
#include "Character/CharacterBase.h"
#include "AbilitySystem/Components/AbilityAimComponent.h"



UCastedAbility::UCastedAbility(const FObjectInitializer &f) :
    Super(f)
{
    OnPrecastFinish.BindUObject(this,&UCastedAbility::PrecastFinish);
    OnCastFinish.BindUObject(this,&UCastedAbility::CastFinish);
    OnCastReadyFinish.BindUObject(this,&UCastedAbility::CastReadyFinish);
    OnReleaseFinish.BindUObject(this,&UCastedAbility::ReleaseFinish);
}

void UCastedAbility::AbilityInit()
{
    CurrentCharges = MaxCharges;

    Super::AbilityInit();
}

void UCastedAbility::AbilitySetup(ACharacter *Component)
{
    Super::AbilitySetup(Component);
}

void UCastedAbility::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if(bIsCasted)
    {
        CurrentCastTime += DeltaTime;
        if(CurrentCastTime >= CastTime && !bCastReady)
        {
            bCastReady = true;
            bUserAnimInterrupt = true;
            StartAnimation(CastReadySectionName,OnCastReadyFinish);
        }
    }
}

void UCastedAbility::PrecastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(!bInterrupted)
    {
        CurrentCastTime = 0;
        bIsCasted = true;
        CastNotify();
        StartAnimation(CastLoopSectionName,OnCastFinish);
    }
    else
    {
        Interrupt_deprecated();
        UE_LOG(AbilitySystemLog, Warning, TEXT("Casted Ability: Interrupted"));
    }
}

void UCastedAbility::CastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(bUserAnimInterrupt)// to cast ready then
        bUserAnimInterrupt = false;
    else if(bInterrupted)
        Interrupt_deprecated();
}

void UCastedAbility::CastReadyFinish(UAnimMontage* animMontage, bool bInterrupted)
{
    if(bUserAnimInterrupt)// to ability ready then
    {
        bUserAnimInterrupt = false;
        AbilityReady();
    }
    else if(bInterrupted)
    {
        Interrupt_deprecated();
    }
    bCastReady = false;
}

void UCastedAbility::ReleaseFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(!bInterrupted)
    {
        ReleaseNotify();
        Cancel_deprecated();
    }
    else
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Casted Ability: interrupted"));
        Interrupt_deprecated();
    }
}

void UCastedAbility::Activate_deprecated()
{
    if(StartAnimation(PrecastSectionName,OnPrecastFinish))
    {
        StopCooldown();
        //StartCooldown();
    }
    else
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Casted Ability: OnActivate No Cast Montage"));
        return;
    }
    bIsActive = true;
    Super::Activate_deprecated();
}

void UCastedAbility::Release_deprecated()
{
    if((!bIsCasted) || (CurrentCastTime < CastTime))
    {
        Cancel_deprecated();// character ended
        return;
    }

    bIsCasted = false;
    bUserAnimInterrupt = true;
    StartAnimation(AftercastSectionName,OnReleaseFinish);

    --CurrentCharges;
    Super::Release_deprecated();
}

void UCastedAbility::Cancel_deprecated()
{
    bIsCasted = false;
    StopAnimation();
    if(!bIsCooldown)
        StartCooldown();
    Super::Cancel_deprecated();
}

void UCastedAbility::Interrupt_deprecated()
{
    bIsCasted = false;
    StopAnimation();
    StartCooldown();
    Super::Interrupt_deprecated();
}

void UCastedAbility::CastNotify()
{
    OnCastNotify();
}

void UCastedAbility::ReleaseNotify()
{
    OnReleaseNotify();
}

void UCastedAbility::AbilityReady()
{
    OnAbilityReady();
}





