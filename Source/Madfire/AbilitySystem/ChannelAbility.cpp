#include "ChannelAbility.h"
#include "Character/CharacterBase.h"
#include "AbilitySystemLog.h"


UChannelAbility::UChannelAbility(const FObjectInitializer &f) :
    Super(f)
{

}

void UChannelAbility::AbilityInit()
{
    CurrentCharges = MaxCharges;
    TargetSelection = NewObject<UAbilityTargetSelectionComponent>(this,TargetSelectionClass, "TargetSelectionComponent");
    if(TargetSelection)
        AddComponent(TargetSelection);

    OnPrecastFinish.BindUObject(this,&UChannelAbility::PrecastFinish);
    OnCastFinish.BindUObject(this,&UChannelAbility::CastFinish);

    if(TargetSelection)
    {
        TargetSelection->OnTargetChoosed.AddDynamic(this, &UChannelAbility::Execute);
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Channel Ability: Can't create Target Selection Algorithm"));
    }
}

void UChannelAbility::AbilitySetup(UAbilitySystemComponent *Component)
{
    Super::AbilitySetup(Component);
}

bool UChannelAbility::StartAnimation(FName SectionName, FOnMontageBlendingOutStarted Delegate)
{
    if(CastAnimMontage)
    {
        if(AbilitySystemComponent && AbilitySystemComponent->GetOwner())
        {
            auto Owner = Cast<ACharacterBase>(AbilitySystemComponent->GetOwner());
            if(Owner)
            {
                Owner->PlayAnimMontage(CastAnimMontage,1.f,SectionName);//PrecastSectionName);

                auto animInstance    = Owner->GetMesh()->GetAnimInstance();
                auto montageInstance = animInstance->GetActiveInstanceForMontage(CastAnimMontage);
                montageInstance->OnMontageBlendingOutStarted = Delegate;//OnPrecastFinish;

                return true;
            }
            else
            {
                UE_LOG(AbilitySystemLog, Error, TEXT("Channel Ability: Owner is not CharacterBase"));
            }
        }
        else
        {
            UE_LOG(AbilitySystemLog, Error, TEXT("Channel Ability: AbilitySystemComponent is not set"));
        }
    }
    return false;
}

void UChannelAbility::PrecastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(!bInterrupted)
    {
        TargetSelection->ActivateComponent();
        StartAnimation(CastLoopSectionName,OnCastFinish);
        GetWorld()->GetTimerManager().SetTimer(FuzeTimerHandle,this,&UChannelAbility::ChannelUpdate, CastRate, true);
    }
    else
    {
        AbilitySystemComponent->SetSound(AbilitySound,false);
        Cast<ACharacter>(AbilitySystemComponent->GetOwner())->StopAnimMontage(CastAnimMontage);

        UE_LOG(AbilitySystemLog, Warning, TEXT("Channel Ability: Interrupted"));
    }
}

void UChannelAbility::CastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    TargetSelection->DeactivateComponent();
    auto Owner = Cast<ACharacterBase>(AbilitySystemComponent->GetOwner());
    if (Owner)
    {
        if (!bInterrupted)
        {            
            // PlayAftercast
            // Note loop section is always interrupted
        }
        else
        {
            UE_LOG(AbilitySystemLog, Warning, TEXT("Channel Ability: Interrupted"));
        }
    }
}

void UChannelAbility::ChannelUpdate_Implementation()
{
    TargetSelection->UpdateComponent();
    if(CurrentCharges < 1)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Not enough Charges"));
        StartCooldown();
        OnRelease();
    }
    else
        --CurrentCharges;
}

void UChannelAbility::OnActivate()
{
    StopCooldown();

    if(AbilitySound)
    {
        AbilitySystemComponent->SetSound(AbilitySound,true);
    }

    if(!StartAnimation(PrecastSectionName,OnPrecastFinish))
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Channel Ability: OnActivate No Cast Montage"));

        GetWorld()->GetTimerManager().SetTimer(FuzeTimerHandle,this,&UChannelAbility::ChannelUpdate,CastRate,true);
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Channel Ability: No Target Selection Algorithm"));
    }
}

void UChannelAbility::OnRelease()
{
    if(TargetSelection)
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Channel Ability: OnDeactivate"));
        TargetSelection->DeactivateComponent();
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Channel Ability: No Target Selection Algorithm"));
    }

    auto Owner = Cast<ACharacterBase>(AbilitySystemComponent->GetOwner());
    Owner->PlayAnimMontage(CastAnimMontage, 1.f, AftercastSectionName);

    OnCancel();
}

void UChannelAbility::OnCancel()
{
    if(AbilitySound && AbilitySystemComponent)
    {
        AbilitySystemComponent->SetSound(AbilitySound,false);
    }

    GetWorld()->GetTimerManager().ClearTimer(FuzeTimerHandle);
    StartCooldown();
    bIsActivated = false;
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

