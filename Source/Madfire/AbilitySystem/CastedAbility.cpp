#include "CastedAbility.h"
#include "Character/CharacterBase.h"
#include "AbilitySystem/AbilityComponents/AbilityAimComponent.h"
#include "AbilitySystemLog.h"



UCastedAbility::UCastedAbility(const FObjectInitializer &f) :
    Super(f)
{}

void UCastedAbility::AbilityInit()
{
    CurrentCharges = MaxCharges;

    OnPrecastFinish.BindUObject(this,&UCastedAbility::PrecastFinish);
    OnCastFinish.BindUObject(this,&UCastedAbility::CastFinish);
    OnCastReadyFinish.BindUObject(this,&UCastedAbility::CastReadyFinish);
    OnReleaseFinish.BindUObject(this,&UCastedAbility::ReleaseFinish);
}

void UCastedAbility::AbilitySetup(UAbilitySystemComponent *Component)
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
    // todo think of a way to separate interrupt from cast ready progress and call cancel when interrupt
}

void UCastedAbility::CastReadyFinish(UAnimMontage* animMontage, bool bInterrupted)
{
    bCastReady = false;
    AbilityReady();
}

void UCastedAbility::ReleaseFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(!bInterrupted)
    {
        ReleaseNotify();
        StartCooldown();
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
    Super::Activate();
}

void UCastedAbility::Release()
{
    if((!bIsCasted) || (CurrentCastTime <= CastTime))
    {
        Cancel();
        return;
    }

    bIsCasted = false;

    StartAnimation(AftercastSectionName,OnReleaseFinish);

    --CurrentCharges;
}

void UCastedAbility::Cancel()
{
    Super::Cancel();
    bIsCasted = false;
    StopAnimation();
    StartCooldown();
}

void UCastedAbility::Interrupt()
{
    Super::Interrupt();
    bIsCasted = false;
    StopAnimation();
    StartCooldown();
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





