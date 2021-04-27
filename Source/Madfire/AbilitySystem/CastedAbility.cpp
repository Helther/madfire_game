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
    if(TargetSelectionClass)
        TargetSelectionComp = NewObject<UAbilityTargetSelectionComponent>(this,TargetSelectionClass, "TargetSelectionComponent");
    if(AimAlgorithmClass)
        AimComp = NewObject<UAbilityAimComponent>(this, AimAlgorithmClass, "AimComponent");

    if(TargetSelectionComp)
        AddComponent(TargetSelectionComp);
    if(AimComp)
        AddComponent(AimComp);

    OnPrecastFinish.BindUObject(this,&UCastedAbility::PrecastFinish);
    OnCastFinish.BindUObject(this,&UCastedAbility::CastFinish);
    OnReleaseFinish.BindUObject(this,&UCastedAbility::ReleaseFinish);

    if(TargetSelectionComp)
        TargetSelectionComp->OnTargetChoosed.AddDynamic(this, &UCastedAbility::Execute);
}

void UCastedAbility::AbilitySetup(UAbilitySystemComponent *Component)
{
    Super::AbilitySetup(Component);
}

void UCastedAbility::Tick(float DeltaTime)
{
    if(bIsCasted)
    {
        CurrentCastTime += DeltaTime;
    }
}

void UCastedAbility::PrecastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    CurrentCastTime = 0;

    if(!bInterrupted)
    {
        if(AimComp)
            AimComp->ActivateComponent();
        StartAnimation(CastLoopSectionName,OnCastFinish);
    }
    else
    {
        AbilitySystemComponent->SetSound(AbilitySound,false);
        Cast<ACharacter>(AbilitySystemComponent->GetOwner())->StopAnimMontage(CastAnimMontage);
        UE_LOG(AbilitySystemLog, Warning, TEXT("Casted Ability: Interrupted"));
    }
}

void UCastedAbility::CastFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(AimComp)
        AimComp->DeactivateComponent();
}

void UCastedAbility::ReleaseFinish(UAnimMontage *animMontage, bool bInterrupted)
{
    if(!bInterrupted)
        TargetSelectionComp->ActivateComponent();
    else
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Casted Ability: interrupted"));
        OnCancel();
    }
}

void UCastedAbility::OnActivate()
{
    StopCooldown();
    CurrentCastTime = 0;
    bIsCasted = true;

    if(AbilitySound && AbilitySystemComponent)
    {
        AbilitySystemComponent->SetSound(AbilitySound,true);
    }

    if(!StartAnimation(PrecastSectionName,OnPrecastFinish))
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Casted Ability: OnActivate No Cast Montage"));
    }
}

void UCastedAbility::OnRelease()
{
    bIsCasted = false;
    if(CurrentCastTime <= CastTime)
    {
        OnCancel();
        return;
    }

    StartAnimation(AftercastSectionName,OnReleaseFinish);

    if(CurrentCharges < 1)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Not enough Charges"));
        StartCooldown();
    }
    else
        --CurrentCharges;
}

void UCastedAbility::OnCancel()
{
    bIsCasted = false;
    AbilitySystemComponent->SetSound(AbilitySound,false);
    Cast<ACharacter>(AbilitySystemComponent->GetOwner())->StopAnimMontage(CastAnimMontage);
    StartCooldown();
}

bool UCastedAbility::StartAnimation(FName SectionName, FOnMontageBlendingOutStarted Delegate)
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


