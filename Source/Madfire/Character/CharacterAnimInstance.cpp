#include "CharacterAnimInstance.h"

#include "Character/CharacterPlayer.h"

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    CharacterPlayer = Cast<ACharacterPlayer>(GetOwningActor());
    Super::NativeUpdateAnimation(DeltaSeconds);
}

void UCharacterAnimInstance::OnWeaponSwitch()
{    
    auto& stateInfo = CharacterPlayer->CombatComponent->CombatFSM.state(ECombatState::WeaponSwitch)->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::WeaponGrabbed, true);
}

void UCharacterAnimInstance::OnBlockActivation(bool activate)
{
    if (CharacterPlayer->CombatComponent->CombatFSM.currentState()->Id == ECombatState::Block)
    {
        auto& stateInfo = CharacterPlayer->CombatComponent->CombatFSM.state(ECombatState::Block)->Info;
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::BlockOrParryActive, activate);
    }
}

void UCharacterAnimInstance::OnParryActivation(bool activate)
{
    if (CharacterPlayer->CombatComponent->CombatFSM.currentState()->Id == ECombatState::Parry)
    {
        auto& stateInfo = CharacterPlayer->CombatComponent->CombatFSM.state(ECombatState::Parry)->Info;
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::BlockOrParryActive, activate);
    }
}

void UCharacterAnimInstance::OnMovementAllowed(bool activate)
{
    auto& stateInfo = CharacterPlayer->CombatComponent->CombatFSM.currentState()->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::MovementBlocked, !activate);
    CharacterPlayer->GetMovementComponent()->MovementLock = !activate;
}

void UCharacterAnimInstance::OnRotationAllowed(bool activate)
{
    auto& stateInfo = CharacterPlayer->CombatComponent->CombatFSM.currentState()->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::RotationBlocked, !activate);
    RootMotionMode = activate ? ERootMotionMode::IgnoreRootMotion : ERootMotionMode::RootMotionFromMontagesOnly;
}

void UCharacterAnimInstance::OnComboCanContinue()
{
    auto& stateInfo = CharacterPlayer->CombatComponent->CombatFSM.state(ECombatState::Attack)->Info;
    if (CharacterPlayer->CombatComponent->CombatFSM.currentState()->Id == ECombatState::Attack)
    {
        if (!stateInfo.CheckAnimInfo(ECombatStateAnimInfo::ComboSaved))
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::ComboCanContinue, true);
        else
            CharacterPlayer->CombatComponent->StartNextComboAttack(false);
    }
}

bool UCharacterAnimInstance::IsMovementAllowed() const
{
    if(CharacterPlayer != nullptr && CharacterPlayer->CombatComponent->CombatFSM.currentState())
    {
        const auto& stateInfo = CharacterPlayer->CombatComponent->CombatFSM.currentState()->Info;
        return !stateInfo.CheckAnimInfo(ECombatStateAnimInfo::MovementBlocked);
    }
    return false;
}
