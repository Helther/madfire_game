#include "AbstractAbility.h"
#include "AbilitySystemLog.h"
#include "Character/Components/CombatComponent.h"

UAbstractAbility::UAbstractAbility(const FObjectInitializer &f) :
    Super(f)
{}

void UAbstractAbility::AbilitySetup(UCombatComponent *Component)
{
    CombatComponent = Component;
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
    Activate();
}

void UAbstractAbility::ReleaseAbility()
{
    if(CurrentCharges <= 0)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: Cooldown"));
        return;
    }
    Release();
}

void UAbstractAbility::CancelAbility()
{
    Cancel();
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
                if(CurrentCharges > MaxCharges)
                    CurrentCharges = MaxCharges;
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

void UAbstractAbility::AbilityInit()
{
    OnAbilityInit();
}

void UAbstractAbility::Activate()
{
    OnAbilityStart();
}

void UAbstractAbility::Release()
{
    OnAbilityEnd();
}

void UAbstractAbility::Cancel()
{
    bIsActive = false;
    OnAbilityCancel();
    check(CombatComponent && "Cancel: Bad component!");
    CombatComponent->UpdateCombatFSM();
}

void UAbstractAbility::Interrupt()
{
    bIsActive = false;
    OnAbilityInterrupt();
    check(CombatComponent && "Interrupt: Bad component!");
    CombatComponent->UpdateCombatFSM();
}

bool UAbstractAbility::StopAnimation()
{
    if(!CombatComponent)
        return false;

    auto Owner = Cast<ACharacter>(CombatComponent->GetOwner());
    if(!Owner)
        return false;

    Owner->StopAnimMontage(CastAnimMontage);
    return true;
}

bool UAbstractAbility::StartAnimation(FName SectionName, FOnMontageBlendingOutStarted Delegate)
{
    if(!CastAnimMontage)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: No CastAnimMontage is not set"));
        return false;
    }

    if(!CombatComponent)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: AbilitySystemComponent is not set"));
        return false;
    }

    if(!CombatComponent->GetOwner())
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: AbilitySystemComponent Owner is not set"));
        return false;
    }


    auto Owner = Cast<ACharacter>(CombatComponent->GetOwner());

    if(!Owner)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: Owner is not CharacterBase"));
        return false;
    }

    Owner->PlayAnimMontage(CastAnimMontage,1.f,SectionName);//PrecastSectionName);

    auto animInstance    = Owner->GetMesh()->GetAnimInstance();
    auto montageInstance = animInstance->GetActiveInstanceForMontage(CastAnimMontage);
    montageInstance->OnMontageBlendingOutStarted = Delegate;//OnPrecastFinish;

    return true;
}
