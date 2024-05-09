#include "ChannelAbility.h"
#include "Character/CharacterBase.h"
//#include "AbilitySystemLog.h"


UChannelAbility::UChannelAbility(const FObjectInitializer &f) :
    Super(f)
{
    OnPrecastFinish.BindUObject(this,&UChannelAbility::PrecastFinish);
    OnCastFinish.BindUObject(this,&UChannelAbility::CastFinish);
}

void UChannelAbility::AbilityInit()
{
    CurrentCharges = MaxCharges;

    Super::AbilityInit();
}

void UChannelAbility::AbilitySetup(ACharacter *Component)
{
    Super::AbilitySetup(Component);
}

void UChannelAbility::PrecastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(!bInterrupted)
    {
        CastNotify();
        StartAnimation(CastLoopSectionName,OnCastFinish);
        GetWorld()->GetTimerManager().SetTimer(FuzeTimerHandle,this,&UChannelAbility::ChannelUpdate, CastRate, true);
    }
    else
    {
        StopAnimation();
        StartCooldown();
        Interrupt();
    }
}

void UChannelAbility::CastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if (bUserAnimInterrupt)
    {
        bUserAnimInterrupt = false;
        // PlayAftercast
        // Note loop section is always interrupted
    }
    else
        Interrupt();
}

void UChannelAbility::ChannelUpdate()
{
    OnChannelUpdate();
    if(CurrentCharges < 1)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Not enough Charges"));
        bUserAnimInterrupt = true;
        StartCooldown();
        Release();
    }
    else
        --CurrentCharges;    
}

void UChannelAbility::Activate()
{    
    if(StartAnimation(PrecastSectionName,OnPrecastFinish))
    {
        StopCooldown();
        GetWorld()->GetTimerManager().SetTimer(FuzeTimerHandle,this,&UChannelAbility::ChannelUpdate,CastRate,true);
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Channel Ability: Can't start the animation"));
        return;
    }
    bIsActive = true;
    Super::Activate();
}

void UChannelAbility::Release()
{
    StartAnimation(AftercastSectionName,OnReleaseFinish);
    Super::Release();
    Cancel();// character ended
}

void UChannelAbility::Cancel()
{
    AbilityEnd();
    Super::Cancel();
}

void UChannelAbility::Interrupt()
{
    AbilityEnd();
    Super::Interrupt();
}

void UChannelAbility::AbilityEnd()
{
    GetWorld()->GetTimerManager().ClearTimer(FuzeTimerHandle);
    StopAnimation();
    if(!bIsCooldown)
        StartCooldown();
    bIsActivated = false;
}

void UChannelAbility::CastNotify()
{
    OnCastNotify();
}

/*
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
            //auto Obj = UAbilityManager::SpawnComponent(Caster, component, 0);
        }
        --CurrentCharges;
    }
}*/

