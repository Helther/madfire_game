#include "CharacterAnimInstance.h"

#include "Character/CharacterCreature.h"

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    CharacterInstance = Cast<ACharacterCreature>(GetOwningActor());
    Super::NativeUpdateAnimation(DeltaSeconds);
}

void UCharacterAnimInstance::OnWeaponSwitch()
{    
    auto& stateInfo = CharacterInstance->CombatComponent->CombatFSM.state(ECombatState::WeaponSwitch)->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::WeaponGrabbed, true);
    CharacterInstance->CombatComponent->WeaponsSetup(!CharacterInstance->CombatComponent->WeaponDrawn());
}

void UCharacterAnimInstance::OnBlockActivation(bool activate)
{
    if (CharacterInstance->CombatComponent->CombatFSM.currentState()->Id == ECombatState::Block)
    {
        auto& stateInfo = CharacterInstance->CombatComponent->CombatFSM.state(ECombatState::Block)->Info;
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::BlockOrParryActive, activate);
    }
}

void UCharacterAnimInstance::OnParryActivation(bool activate)
{
    if (CharacterInstance->CombatComponent->CombatFSM.currentState()->Id == ECombatState::Parry)
    {
        auto& stateInfo = CharacterInstance->CombatComponent->CombatFSM.state(ECombatState::Parry)->Info;
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::BlockOrParryActive, activate);
    }
}

void UCharacterAnimInstance::OnMovementAllowed(bool activate)
{
    auto& stateInfo = CharacterInstance->CombatComponent->CombatFSM.currentState()->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::MovementBlocked, !activate);
    CharacterInstance->GetMovementComponent()->MovementLock = !activate;
}

void UCharacterAnimInstance::OnRotationAllowed(bool activate)
{
    auto& stateInfo = CharacterInstance->CombatComponent->CombatFSM.currentState()->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::RotationBlocked, !activate);
    RootMotionMode = activate ? ERootMotionMode::IgnoreRootMotion : ERootMotionMode::RootMotionFromMontagesOnly;
}

void UCharacterAnimInstance::OnComboCanContinue()
{
    auto& stateInfo = CharacterInstance->CombatComponent->CombatFSM.state(ECombatState::Attack)->Info;
    if (CharacterInstance->CombatComponent->CombatFSM.currentState()->Id == ECombatState::Attack)
    {
        if (!stateInfo.CheckAnimInfo(ECombatStateAnimInfo::ComboSaved))
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::ComboCanContinue, true);
        else
            CharacterInstance->CombatComponent->StartNextComboAttack(false);
    }
}

void UCharacterAnimInstance::OnCollisionActivation(bool mainHand, bool activate)
{
    if (CharacterInstance->CombatComponent->CombatFSM.currentState()->Id == ECombatState::Attack)
    {
        auto& stateInfo = CharacterInstance->CombatComponent->CombatFSM.state(ECombatState::Attack)->Info;
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::CollisionOn, activate);
        CharacterInstance->CombatComponent->SetCurrentWeaponCollision(mainHand, activate);// TODO separate norifies for main and off hand
    }
}

void UCharacterAnimInstance::OnParriableActivation(bool activate)
{
    if (CharacterInstance->CombatComponent->CombatFSM.currentState()->Id == ECombatState::Attack)
    {
        auto& stateInfo = CharacterInstance->CombatComponent->CombatFSM.state(ECombatState::Attack)->Info;
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::CanBeParried, activate);
    }
}

bool UCharacterAnimInstance::IsMovementAllowed() const
{
    if(CharacterInstance != nullptr && CharacterInstance->CombatComponent != nullptr && CharacterInstance->CombatComponent->CombatFSM.currentState())
    {
        const auto& stateInfo = CharacterInstance->CombatComponent->CombatFSM.currentState()->Info;
        return !stateInfo.CheckAnimInfo(ECombatStateAnimInfo::MovementBlocked);
    }
    return false;
}
