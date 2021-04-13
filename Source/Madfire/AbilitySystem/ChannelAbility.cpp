#include "ChannelAbility.h"
#include "Character/CharacterBase.h"
#include "AbilitySystemLog.h"


UChannelAbility::UChannelAbility(const FObjectInitializer &f) :
    Super(f)
{    
}

void UChannelAbility::AbilitySetup(UAbilitySystemComponent *Component)
{
    UE_LOG(AbilitySystemLog, Warning, TEXT("Channel Ability: Setup"));

    TargetSelection = NewObject<UAbilityTargetSelectionComponent>(this,TargetSelectionClass, "TargetSelectionComponent");
    AddComponent(TargetSelection);

    Super::AbilitySetup(Component);

    if(TargetSelection)
    {
        TargetSelection->OnTargetChoosed.AddDynamic(this, &UChannelAbility::Execute);
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Channel Ability: Can't create Target Selection Algorithm"));
    }

    OnPrecastFinish.BindUObject(this,&UChannelAbility::PrecastFinish);
    OnCastFinish.BindUObject(this,&UChannelAbility::CastFinish);
}

void UChannelAbility::PrecastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(!bInterrupted)
    {
        //PlayAnimation;
        if(AbilitySystemComponent && AbilitySystemComponent->GetOwner())
        {
            auto Owner = Cast<ACharacterBase>(AbilitySystemComponent->GetOwner());
            if(Owner)
            {
                Owner->PlayAnimMontage(CastAnimMontage,1.f,CastLoopSectionName);

                auto animInstance    = Owner->GetMesh()->GetAnimInstance();
                auto montageInstance = animInstance->GetActiveInstanceForMontage(CastAnimMontage);
                montageInstance->OnMontageBlendingOutStarted = OnCastFinish;
            }
        }

        TargetSelection->ActivateComponent();
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Channel Ability: Interrupted"));
    }
}

void UChannelAbility::CastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    TargetSelection->DeactivateComponent();

    if(!bInterrupted)
    {
        //PlayAftercast
    }
    else
    {
        //Interrupt
    }
}

void UChannelAbility::OnActivate()
{
    if(CastAnimMontage)
    {
        if(AbilitySystemComponent && AbilitySystemComponent->GetOwner())
        {
            auto Owner = Cast<ACharacterBase>(AbilitySystemComponent->GetOwner());
            if(Owner)
            {
                Owner->PlayAnimMontage(CastAnimMontage,1.f,PrecastSectionName);

                auto animInstance    = Owner->GetMesh()->GetAnimInstance();
                auto montageInstance = animInstance->GetActiveInstanceForMontage(CastAnimMontage);
                montageInstance->OnMontageBlendingOutStarted = OnPrecastFinish;
            }
            else
            {
                UE_LOG(AbilitySystemLog, Error, TEXT("Channel Ability: Owner is not CharacterBase"));
            }
        }
    }
    else if(TargetSelection)
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Channel Ability: OnActivate No Cast Montage"));
        TargetSelection->ActivateComponent();
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
    OnCancel();
}

void UChannelAbility::OnCancel()
{
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

