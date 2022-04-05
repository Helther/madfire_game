#include "CastedAbility.h"
#include "Character/CharacterBase.h"
#include "AbilitySystem/AbilityComponents/AbilityAimComponent.h"
#include "AbilitySystemLog.h"



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

void UCastedAbility::AbilitySetup(UCombatComponent *Component)
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
        Interrupt();
        UE_LOG(AbilitySystemLog, Warning, TEXT("Casted Ability: Interrupted"));
    }
}

void UCastedAbility::CastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(bUserAnimInterrupt)// to cast ready then
        bUserAnimInterrupt = false;
    else if(bInterrupted)
        Interrupt();
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
        Interrupt();
    }
    bCastReady = false;
}

void UCastedAbility::ReleaseFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(!bInterrupted)
    {
        ReleaseNotify();
        Cancel();
    }
    else
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Casted Ability: interrupted"));
        Interrupt();
    }
}

void UCastedAbility::Activate()
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
    Super::Activate();
}

void UCastedAbility::Release()
{
    if((!bIsCasted) || (CurrentCastTime < CastTime))
    {
        Cancel();// character ended
        return;
    }

    bIsCasted = false;
    bUserAnimInterrupt = true;
    StartAnimation(AftercastSectionName,OnReleaseFinish);

    --CurrentCharges;
    Super::Release();
}

void UCastedAbility::Cancel()
{
    bIsCasted = false;
    StopAnimation();
    if(!bIsCooldown)
        StartCooldown();
    Super::Cancel();
}

void UCastedAbility::Interrupt()
{
    bIsCasted = false;
    StopAnimation();
    StartCooldown();
    Super::Interrupt();
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





