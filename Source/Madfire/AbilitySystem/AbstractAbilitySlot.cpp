// Fill out your copyright notice in the Description page of Project Settings.

#include "AbstractAbilitySlot.h"
/*
void UAbstractAbilitySlot::InitSpell(ACharacter *_caster,
                                     TSubclassOf<UAbstractAbility> ability)
{
    if (!ability)
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Slot: shit was fucked"));
        return;
    }

    AbilityReference = ability.GetDefaultObject();

    if (AbilityReference == nullptr)
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Slot: skill is not valid!"));
        return;
    }

    if (!_caster)
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Slot: caster is not valid!"));
        return;
    }

    AbilityReference->Caster = _caster;
    Caster                   = _caster;

    //for debug only
    AbilityReference->CurrentCharges = AbilityReference->MaxCharges;
}

void UAbstractAbilitySlot::KeyPressed()
{
    if (AbilityReference != nullptr && !AbilityReference->isCasting)
    {
        AbilityReference->OnEventKeyPressed();
    }
}

void UAbstractAbilitySlot::KeyReleased()
{
    if (AbilityReference != nullptr)
    {
        AbilityReference->OnEventKeyReleased();
    }
}

void UAbstractAbilitySlot::GlobalCooldown() {}

float UAbstractAbilitySlot::Cooldown()
{
    if (AbilityReference != nullptr)
    {
        AbilityReference->onCooldown = false;
        return AbilityReference->ChargeCDTime;
    }
    else
    {
        return 0.0f;
    }
}

bool UAbstractAbilitySlot::RestoreCharge()
{
    if (AbilityReference != nullptr)
    {
        if (AbilityReference->CurrentCharges < AbilityReference->MaxCharges - 1)
        {
            ++(AbilityReference->CurrentCharges);
            UE_LOG(AbilitySystemLog, Warning, TEXT("Slot: charge was restored!"));
            return false;
        }
        else
        {
            ++(AbilityReference->CurrentCharges);
            UE_LOG(AbilitySystemLog, Warning, TEXT("Slot: last charge was restored!"));
            return true;
        }
    }
    else
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Slot: last charge was restored!"));
        return true;
    }
}
 *
 */
