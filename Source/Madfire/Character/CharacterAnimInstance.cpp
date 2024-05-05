#include "CharacterAnimInstance.h"

#include "Character/CharacterCreature.h"

const FName FAnimSectionNames::Start = "Start";
const FName FAnimSectionNames::End = "End";
const FName FAnimSectionNames::Loop = "Loop";
const FName FAnimSectionNames::Cancel = "Cancel";


void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    CharacterInstance = Cast<ACharacterCreature>(GetOwningActor());
    Super::NativeUpdateAnimation(DeltaSeconds);
}

void UCharacterAnimInstance::OnWeaponSwitch()
{    
    auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().state(ECombatState::WeaponSwitch)->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::WeaponGrabbed, true);
    CharacterInstance->GetCombatComponent()->GetCurrentCombatStance()->ArmsSetup(!CharacterInstance->GetCombatComponent()->WeaponDrawn());
}

void UCharacterAnimInstance::OnAmmoGrab()
{
    auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().state(ECombatState::Reload)->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AmmoGrabbed, true);
    Cast<URangeCombatStance>(CharacterInstance->GetCombatComponent()->GetCurrentCombatStance())->SpawnAttachAmmoActor();
}

void UCharacterAnimInstance::OnAmmoAttach()
{
    auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().state(ECombatState::Reload)->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AmmoAttach, true);
    Cast<URangeCombatStance>(CharacterInstance->GetCombatComponent()->GetCurrentCombatStance())->AttachAmmoActorToWeapon();
}

void UCharacterAnimInstance::OnProjectileFired()
{
    CharacterInstance->FireLoadedProjectile();
}

void UCharacterAnimInstance::OnBlockActivation(bool activate)
{
    if (CharacterInstance->GetCombatComponent()->GetCombatFSM().isInState(ECombatState::Block))
    {
        auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().state(ECombatState::Block)->Info;
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::BlockOrParryActive, activate);
    }
}

void UCharacterAnimInstance::OnParryActivation(bool activate)
{
    if (CharacterInstance->GetCombatComponent()->GetCombatFSM().isInState(ECombatState::Parry))
    {
        auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().state(ECombatState::Parry)->Info;
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::BlockOrParryActive, activate);
    }
}

void UCharacterAnimInstance::OnMovementAllowed(bool activate)
{
    auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().currentState()->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::MovementBlocked, !activate);
    CharacterInstance->GetMovementComponent()->MovementLock = !activate;
}

void UCharacterAnimInstance::OnRotationAllowed(bool activate)
{
    auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().currentState()->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::RotationBlocked, !activate);
    RootMotionMode = activate ? ERootMotionMode::IgnoreRootMotion : ERootMotionMode::RootMotionFromMontagesOnly;
    if (activate)
        CharacterInstance->GetCombatComponent()->ResetRotationRate();
    else
        CharacterInstance->GetMovementComponent()->RotationRate.Yaw = 0; // remove rotation speed for animations that don't have root motion
}

void UCharacterAnimInstance::OnComboCanContinue()
{
    auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().state(ECombatState::Attack)->Info;
    if (CharacterInstance->GetCombatComponent()->GetCombatFSM().isInState(ECombatState::Attack))
    {
        if (!stateInfo.CheckAnimInfo(ECombatStateAnimInfo::ComboSaved))
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::ComboCanContinue, true);
        else
            CharacterInstance->GetCombatComponent()->StartNextComboAttack(false);
    }
}

void UCharacterAnimInstance::OnCollisionActivation(EArmsSlot WeaponSlot, bool activate)
{
    auto combatStance = CharacterInstance->GetCombatComponent()->GetCurrentCombatStance();
    if (!combatStance)
        return;

    ECombatState state = combatStance->IsMelee() ? ECombatState::Attack : ECombatState::Parry;  // ranged attack can come from parry only
    if (CharacterInstance->GetCombatComponent()->GetCombatFSM().isInState(state))
    {
        auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().state(state)->Info;
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::CollisionOn, activate);
        if (activate)
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, false);
        AItemActor* weaponActor = CharacterInstance->GetCombatComponent()->GetCurrentCombatStance()->GetArmsActorFromSlot(WeaponSlot);
        check(weaponActor && "invalid weapon collision anim event");
        UCombatComponent::SetWeaponCollision(weaponActor, activate);
    }
}

void UCharacterAnimInstance::OnParriableActivation(bool activate)
{
    auto combatStance = CharacterInstance->GetCombatComponent()->GetCurrentCombatStance();
    if (!combatStance)
        return;

    ECombatState state = combatStance->IsMelee() ? ECombatState::Attack : ECombatState::Parry;
    if (CharacterInstance->GetCombatComponent()->GetCombatFSM().isInState(state))
    {
        auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().state(state)->Info;
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::CanBeParried, activate);
    }
}

bool UCharacterAnimInstance::IsMovementAllowed() const
{
    if (CharacterInstance != nullptr && CharacterInstance->GetCombatComponent() != nullptr 
        && CharacterInstance->GetCombatComponent()->HasCombatStance()
        && CharacterInstance->GetCombatComponent()->GetCombatFSM().currentState())
    {
        const auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().currentState()->Info;
        return !stateInfo.CheckAnimInfo(ECombatStateAnimInfo::MovementBlocked);
    }
    return false;
}


void UCharacterAnimInstance::OnDodgeInvulnerable(bool activate)
{
    if (CharacterInstance != nullptr && CharacterInstance->GetMovementComponent()->MovementMode == MOVE_Custom && 
        CharacterInstance->GetMovementComponent()->CustomMovementMode == ECustomMovementMode::Dodge)
    {
        CharacterInstance->Cheats.InvulnerableToHit = activate;
    }
}

void UCharacterAnimInstance::OnCounterChamber(bool activate)
{
    auto& stateInfo = CharacterInstance->GetCombatComponent()->GetCombatFSM().currentState()->Info;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::CanCounterChamber, activate);
}
