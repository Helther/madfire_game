#include "AbilitySamples.h"
#include "Character/Components/CombatComponent.h"
#include "AbilitySystem/AbilityFunctional.h"
#include "Character/CharacterBase.h"


void UInstantAbility_Sample::AbilityInit()
{
    Super::AbilityInit();
    SoundComponent = Cast<UAbilitySoundComponent>(UAbilityFunctional::CreateComponent(this,UAbilitySoundComponent::StaticClass()));

    check(SoundComponent != nullptr)
    AddComponent(SoundComponent);

    if(TargetComponent)
    {
        TargetComponent->OnTargetChoosed.AddDynamic(this, &UAbstractAbility::OnTargetExecute);
        AddComponent(TargetComponent);
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("InstantAbility_Sample: No Target Component"));
    }
}

void UInstantAbility_Sample::CastNotify()
{
    if(AbilitySound->IsValidLowLevelFast())
        SoundComponent->PlaySound(AbilitySound);
    if(TargetComponent)
        TargetComponent->ActivateComponent();
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("InstantAbility_Sample: No Target Component"));
    }
    Super::CastNotify();
}


void UCastedAbility_Sample::AbilityInit()
{
    Super::AbilityInit();
    if(AimingComponent)
        AddComponent(AimingComponent);
    if(TargetComponent)
    {
        AddComponent(TargetComponent);
        TargetComponent->OnTargetChoosed.AddDynamic(this, &UAbstractAbility::OnTargetExecute);
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("CastedAbility_Sample: No Target Component"));
    }
}

void UCastedAbility_Sample::CastNotify()
{
    if(AimingComponent && CombatComponent)
        CombatComponent->UseAimComponent(AimingComponent);
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("CastedAbility_Sample: No Aim Component"));
    }
    Super::CastNotify();
}

void UCastedAbility_Sample::Release()
{
    if(AimingComponent)
        AimingComponent->DeactivateComponent();
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("CastedAbility_Sample: No Aim Component"));
    }
    Super::Release();
}

void UCastedAbility_Sample::Interrupt()
{
    if(AimingComponent)
        AimingComponent->DeactivateComponent();
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("CastedAbility_Sample: No Aim Component"));
    }
    Super::Interrupt();
}

void UCastedAbility_Sample::ReleaseNotify()
{
    if(TargetComponent)
        TargetComponent->ActivateComponent();
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("CastedAbility_Sample: No Target Component"));
    }
    Super::ReleaseNotify();
}

void UChannelAbility_Sample::AbilityInit()
{
    Super::AbilityInit();
    if(TargetComponent)
    {
        AddComponent(TargetComponent);
        TargetComponent->OnTargetChoosed.AddDynamic(this, &UAbstractAbility::OnTargetExecute);
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("UChannelAbility_Sample: No Target Component"));
    }
}

void UChannelAbility_Sample::ChannelUpdate()
{
    Super::ChannelUpdate();

    if(TargetComponent)
        TargetComponent->UpdateComponent();
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("UChannelAbility_Sample: No Target Component"));
    }
}

void UChannelAbility_Sample::CastNotify()
{
    if(TargetComponent)
        TargetComponent->ActivateComponent();
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("UChannelAbility_Sample: No Target Component"));
    }
    Super::CastNotify();
}

void UChannelAbility_Sample::Release()
{
    if(TargetComponent)
        TargetComponent->DeactivateComponent();
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("UChannelAbility_Sample: No Target Component"));
    }
    Super::Release();
}

