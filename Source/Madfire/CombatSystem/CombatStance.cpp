#include "CombatStance.h"
#include "../Character/CharacterCreature.h"


UCombatComponent* UCombatStance::GetCombatComp() const
{
   return Owner->CombatComponent;
}

void UCombatStance::StanceSwitched(ACharacterCreature* NewOwner, const UCombatStance* DefaultStance)
{
    Owner = NewOwner;
    UpdateOwnerAnims(DefaultStance);
    OnStanceActivated();
    if (!Owner->CombatComponent->WeaponDrawn())
        ArmsSetup(false); // attach the weapons
}

void UCombatStance::ResetStance()
{
    OnStanceDeActivated();
    ResetWeaponVars();
    CombatFSM.setCurrentState(ECombatState::Idle);
    Owner = nullptr;
}

void UCombatStance::UpdateOwnerAnims(const UCombatStance* DefaultStance)
{
    Owner->AnimInstance->CurrentHitWallAnim = HitWallAnim ? HitWallAnim : DefaultStance->HitWallAnim;
    Owner->AnimInstance->CurrentWasParriedAnim = WasParriedAnim ? WasParriedAnim : DefaultStance->WasParriedAnim;
    Owner->AnimInstance->CurrentBlockedAttackAnim = BlockedAttackAnim ? BlockedAttackAnim : DefaultStance->BlockedAttackAnim;
    Owner->AnimInstance->CurrentChamberAnim = ChamberAnim ? ChamberAnim : DefaultStance->ChamberAnim;
    Owner->UpdateCCLOBAnims();

    Owner->AnimInstance->CurrentCrouchLocomotion = CrouchLocomotion ? CrouchLocomotion : DefaultStance->CrouchLocomotion;
    Owner->AnimInstance->CurrentStandLocomotion = StandLocomotion ? StandLocomotion : DefaultStance->StandLocomotion;;
    Owner->AnimInstance->CurrentAimOffset = AimOffset ? AimOffset : DefaultStance->AimOffset;;
    Owner->AnimInstance->CurrentBlockAnim = BlockAnim ? BlockAnim : DefaultStance->BlockAnim;;
    Owner->AnimInstance->CurrentParryAnim = ParryAnim ? ParryAnim : DefaultStance->ParryAnim;;
    Owner->AnimInstance->CurrentDrawAnim = DrawAnim ? DrawAnim : DefaultStance->DrawAnim;;
    Owner->AnimInstance->CurrentSheathAnim = SheathAnim ? SheathAnim : DefaultStance->SheathAnim;;
}

void UCombatStance::OnAbilityEnter()
{
    Owner->Logger.Log(GetCombatComp(), "Ability enter");
    if (!GetCombatComp()->WeaponDrawn())// not in combat mode
    {
        Owner->GetMovementComponent()->maxSpeedBlock = true;
        Owner->GetMovementComponent()->bUseControllerDesiredRotation = true;
        Owner->GetMovementComponent()->bOrientRotationToMovement = false;
    }
    GetCombatComp()->CalculateMaxCastingSpeed(Owner->GetMovementComponent()->GetSpeedModifier());
    Owner->AttributeComponent->Stamina->Consume(GetCombatComp()->CurrentAbility->StaminaCost);
    Owner->AttributeComponent->Stamina->BlockRegen();
    Owner->GetMovementComponent()->MoveAbilitiesCurrent = GetCombatComp()->CurrentAbility->MoveAbilities;
    GetCombatComp()->CurrentAbility->ActivateAbility();
}

void UCombatStance::OnAbilityLeave()
{
    Owner->Logger.Log(GetCombatComp(), "Ability leave");
    if (GetCombatComp()->WeaponDrawn())// in combat mode
        GetCombatComp()->CalculateMaxCombatSpeeds();
    else
    {
        Owner->GetMovementComponent()->maxSpeedBlock = false;
        Owner->GetMovementComponent()->CalculateMaxSpeeds(Owner->GetMovementComponent()->GetSpeedModifier());
        Owner->GetMovementComponent()->bUseControllerDesiredRotation = false;
        Owner->GetMovementComponent()->bOrientRotationToMovement = true;
    }
    Owner->AttributeComponent->Stamina->UnblockRegen();
    Owner->GetMovementComponent()->MoveAbilitiesCurrent = Owner->GetMovementComponent()->MoveAbilitiesDefault;// TODO think how to restore it to last state
    GetCombatComp()->SetGlobalCooldown();
}

bool UCombatStance::IdleToAbility() const
{ // check if not active and not in cooldown and more
    bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0;
    return GetCombatComp()->IsAbilityPressed() && hasStamina && GetCombatComp()->CurrentAbility.IsValid() && !GetCombatComp()->CurrentAbility->bIsCooldown && !GetCombatComp()->IsOnGlobalCooldown();
}

bool UCombatStance::AbilityToIdle() const
{
    return GetCombatComp()->CurrentAbility != nullptr && !GetCombatComp()->CurrentAbility->bIsActive;
}

FAttackInfo* UMeleeCombatStance::GetAttackInfoForSpecial(ESpecialType type)
{
    return SpecialAttacks.FindByPredicate([type](const FAttackInfo& info) { return info.Special == type; });
}

UMeleeCombatStance::UMeleeCombatStance() : Super()
{
    CombatFSM.addState(ECombatState::Idle).addState(ECombatState::WeaponSwitch).addState(ECombatState::Attack).addState(ECombatState::Block).addState(ECombatState::Parry).addState(ECombatState::Ability);
    CombatFSM.setCurrentState(ECombatState::Idle);
    //=================================== IDLE =====================================//
    CombatFSM.state(ECombatState::Idle)->Info.OnEnter.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), *FString::Printf(TEXT("idle enter: %s"),
                GetCombatComp()->WeaponDrawn() ? TEXT("weapon on") : TEXT("weapon off")));
        GetCombatComp()->ResetRotationRate();
        auto& pendingStance = GetCombatComp()->PendingStance;
        if (pendingStance.IsSet())
            GetCombatComp()->SetNewStance(this, pendingStance.NewStance, pendingStance.DefaultStance);
    });
    CombatFSM.state(ECombatState::Idle)->Info.OnLeave.BindLambda([this]()
    {
        CombatFSM.state(ECombatState::Idle)->Info.ResetAnimInfo();
        Owner->Logger.Log(GetCombatComp(), "idle leave");
        GetCombatComp()->ResetRotationRate();
    });
    FLinkReq req;
    req.BindLambda([this]()
    {
        return CombatFSM.isInState(ECombatState::Idle) && (GetCombatComp()->IsWeaponSwitchPressed() ||
            GetCombatComp()->PendingStance.IsSet());
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::WeaponSwitch, req);
    req.Unbind();
    req.BindLambda([this]()
    {
        bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0;
        bool hasAnim = Owner->AnimInstance->CurrentBlockAnim != nullptr;
        if (!hasAnim)
             Owner->Logger.Log(GetCombatComp(), "Stance doesn't have block anim");
        return hasAnim && IsBlockAllowed && GetCombatComp()->WeaponDrawn() && CombatFSM.isInState(ECombatState::Idle) && GetCombatComp()->IsBlockPressed() && hasStamina;
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::Block, req);
    req.Unbind();
    req.BindLambda([this]() // set relevant attack info into current state, check special, if not start combo
    {
        bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0;
        if (!GetCombatComp()->WeaponDrawn() || !GetCombatComp()->IsAttackPressed() || !hasStamina || Owner->GetMovementComponent()->IsTransitionInProgress())
            return false;
        ESpecialType special = ESpecialType::None;
        bool isDirectional = false;
        if (Owner->GetMovementComponent()->bIsSprinting)
            special = ESpecialType::Sprint;
        else if (Owner->GetMovementComponent()->IsCrouching())
            special = ESpecialType::Crouch;
        else if (Owner->GetMovementComponent()->IsFalling())
            special = ESpecialType::Air;
        else
        {
            isDirectional = true;
            if (GetCombatComp()->SpecialDirections & ESpecialDirection::SFORWARD)
                special = ESpecialType::Forward;
            else if (GetCombatComp()->SpecialDirections & ESpecialDirection::SBACK)
                special = ESpecialType::Back;
            else if (GetCombatComp()->SpecialDirections & ESpecialDirection::SLEFT)
                special = ESpecialType::Left;
            else if (GetCombatComp()->SpecialDirections & ESpecialDirection::SRIGHT)
                special = ESpecialType::Right;
        }
        bool startNewCombo = false;
        if (special != ESpecialType::None)
        {
            CombatFSM.state(ECombatState::Attack)->Info.AttackInfo = GetAttackInfoForSpecial(special);
            // if has no valid directionals then just start combo-attack
            if (CombatFSM.state(ECombatState::Attack)->Info.AttackInfo == nullptr && isDirectional)
                startNewCombo = true;
        }
        else
            startNewCombo = true;
        if (startNewCombo)
        { // may not be any combos
            auto newAttackInfo = ComboAttacks.Num() != 0 ? &ComboAttacks[0].NormalAttack : nullptr;
            CombatFSM.state(ECombatState::Attack)->Info.AttackInfo = newAttackInfo;
        }
        bool result = CombatFSM.state(ECombatState::Attack)->Info.AttackInfo != nullptr && CombatFSM.state(ECombatState::Attack)->Info.AttackInfo->AttackMontage != nullptr;
        if (!result)
            Owner->Logger.Log(GetCombatComp(), "Stance doesn't have the attack info/anim");
        return result;
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::Attack, req);
    req.Unbind();
    req.BindLambda([this]()
    {
        return IdleToAbility();
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::Ability, req);
    req.Unbind();

    //================================ WEAPON SWITCH ==============================//
    CombatFSM.state(ECombatState::WeaponSwitch)->Info.OnEnter.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), "weapon switch enter");
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(GetCombatComp(), &UCombatComponent::OnWeaponSwitchEnded);
        Owner->StartAnimation(GetCombatComp()->WeaponDrawn() ?
            Owner->AnimInstance->CurrentSheathAnim : Owner->AnimInstance->CurrentDrawAnim, callBack, GetCombatComp()->AttackSpeed);
    });
    CombatFSM.state(ECombatState::WeaponSwitch)->Info.OnLeave.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), "weapon switch leave");
        bool drawn = GetCombatComp()->WeaponDrawn();
        if (drawn) // in combat mode
        {
            Owner->GetMovementComponent()->maxSpeedBlock = true;
            GetCombatComp()->CalculateMaxCombatSpeeds();
            Owner->GetMovementComponent()->bUseControllerDesiredRotation = true;
            Owner->GetMovementComponent()->bOrientRotationToMovement = false;
            Owner->GetMovementComponent()->RotationRate.Yaw = GetCombatComp()->CombatModeRotationRate;
            auto& pendingStance = GetCombatComp()->PendingStance;
            if (pendingStance.IsSet())
                pendingStance.Reset();
        }
        else // exit combat mode
        {// reset max speeds
            Owner->GetMovementComponent()->maxSpeedBlock = false;
            Owner->GetMovementComponent()->CalculateMaxSpeeds(Owner->GetMovementComponent()->GetSpeedModifier());
            Owner->GetMovementComponent()->bUseControllerDesiredRotation = false;
            Owner->GetMovementComponent()->bOrientRotationToMovement = true;
        }
        CombatFSM.state(ECombatState::WeaponSwitch)->Info.ResetAnimInfo();
    });
    req.BindLambda([this]()  // to idle after animation complete
    {
        return CombatFSM.state(ECombatState::WeaponSwitch)->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded); 
    });
    CombatFSM.addLink(ECombatState::WeaponSwitch, ECombatState::Idle, req);
    req.Unbind();

    //=================================== Block =====================================//
    CombatFSM.state(ECombatState::Block)->Info.OnEnter.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), "block enter");
        GetCombatComp()->CalculateMaxBlockingSpeed(Owner->GetMovementComponent()->GetSpeedModifier());
        Owner->AttributeComponent->Stamina->RegenCoef += GetCombatComp()->BlockStaminaRegenRateMod; // slowing down stamina regen rate
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(GetCombatComp(), &UCombatComponent::OnBlockEnded);
        Owner->StartAnimation(Owner->AnimInstance->CurrentBlockAnim, callBack, GetCombatComp()->AttackSpeed, FAnimSectionNames::Start);
    });
    CombatFSM.state(ECombatState::Block)->Info.OnLeave.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), "block leave");
         GetCombatComp()->CalculateMaxCombatSpeeds();
        Owner->AttributeComponent->Stamina->RegenCoef -=  GetCombatComp()->BlockStaminaRegenRateMod; // reseting stamina regen rate
        CombatFSM.state(ECombatState::Block)->Info.ResetAnimInfo();
    });
    req.BindLambda([this]() // to idle after animation complete
    {
        const auto state = CombatFSM.state(ECombatState::Block);
        return state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && 
            !state->Info.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt); 
    });
    CombatFSM.addLink(ECombatState::Block, ECombatState::Idle, req);
    req.Unbind();
    req.BindLambda([this]() // check if stance has parry
    {
        bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0;
        const auto state = CombatFSM.state(ECombatState::Block);
        bool hasAnim = Owner->AnimInstance->CurrentParryAnim != nullptr;
        bool isNotFalling = !Owner->GetMovementComponent()->IsFalling();
        if (!hasAnim)
             Owner->Logger.Log(GetCombatComp(), "Stance doesn't have parry anim");
        return hasAnim && IsParryAllowed && isNotFalling && state->Info.CheckAnimInfo(ECombatStateAnimInfo::BlockOrParryActive) &&  GetCombatComp()->IsAttackPressed() && hasStamina;
    });
    CombatFSM.addLink(ECombatState::Block, ECombatState::Parry, req);
    req.Unbind();
    
    //=================================== Parry =====================================//
    CombatFSM.state(ECombatState::Parry)->Info.OnEnter.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), "parry enter");
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(GetCombatComp(), &UCombatComponent::OnParryEnded);
        Owner->StartAnimation(Owner->AnimInstance->CurrentParryAnim, callBack, GetCombatComp()->AttackSpeed);
        Owner->AttributeComponent->Stamina->Consume(ParryStaminaCost);
        Owner->AttributeComponent->Stamina->BlockRegen();
        Owner->GetMovementComponent()->RotationRate.Yaw = GetCombatComp()->ActionRotationRate;
    });
    CombatFSM.state(ECombatState::Parry)->Info.OnLeave.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), *FString::Printf(TEXT("parry leave")));
        GetCombatComp()->ResetActionState(ECombatState::Parry);
    });
    req.BindLambda([this]()  // to idle after animation complete
    {
        const auto state = CombatFSM.state(ECombatState::Parry);
        return state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && !GetCombatComp()->IsBlockPressed();
    });
    CombatFSM.addLink(ECombatState::Parry, ECombatState::Idle, req);
    req.Unbind();
    req.BindLambda([this]()  // to block after animation complete and blocked or manual interrupt
    {
        bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0; // can't start block without stamina
        const auto state = CombatFSM.state(ECombatState::Parry);
        bool checkEnd = state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) &&  GetCombatComp()->IsBlockPressed();
        bool hasAnim = Owner->AnimInstance->CurrentBlockAnim != nullptr;
        if (!hasAnim)
             Owner->Logger.Log(GetCombatComp(), "Stance doesn't have block anim");
        return hasAnim && IsBlockAllowed && hasStamina && checkEnd && !Owner->GetMovementComponent()->IsRootOrTeleport();
    });
    CombatFSM.addLink(ECombatState::Parry, ECombatState::Block, req);
    req.Unbind();
    
    //=================================== Attack ====================================//
    CombatFSM.state(ECombatState::Attack)->Info.OnEnter.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), "attack enter");
        const FAttackInfo* currentAttack = CombatFSM.state(ECombatState::Attack)->Info.AttackInfo;
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(GetCombatComp(), &UCombatComponent::OnAttackEnded);
        Owner->StartAnimation(currentAttack->AttackMontage, callBack, GetCombatComp()->AttackSpeed, currentAttack->AnimSectionName);
        bool isComboAttack = CombatFSM.state(ECombatState::Attack)->Info.AttackInfo->Special == ESpecialType::None;
        bool hasChargeAttack = ComboAttacks[0].ChargeAttackStages.Num() > 0;
        if (isComboAttack && hasChargeAttack)
            CombatFSM.state(ECombatState::Attack)->Info.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, true);
        Owner->AttributeComponent->Stamina->Consume(currentAttack->StaminaCost);
        Owner->AttributeComponent->Stamina->BlockRegen();
        Owner->GetMovementComponent()->RotationRate.Yaw = GetCombatComp()->ActionRotationRate;
         GetCombatComp()->CalculateMaxBlockingSpeed(Owner->GetMovementComponent()->GetSpeedModifier());
    });
    CombatFSM.state(ECombatState::Attack)->Info.OnLeave.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), "attack leave");
        GetCombatComp()->ResetActionState(ECombatState::Attack);
        CombatFSM.state(ECombatState::Attack)->Info.ChargeTime = 0.f;
        CombatFSM.state(ECombatState::Attack)->Info.AttackInfo = nullptr;
        CombatFSM.state(ECombatState::Attack)->Info.ChargeStage = -1;
        ResetComboCount();
        GetCombatComp()->CharactersHit.Reset();
        auto activeWeapons = GetCombatComp()->FindActiveMeleeWeapons();
        for (auto& weaponActor : activeWeapons)
            UCombatComponent::SetWeaponCollision(weaponActor, false);
        GetCombatComp()->CalculateMaxCombatSpeeds();
    });
    req.BindLambda([this]()
    {
        const auto state = CombatFSM.state(ECombatState::Attack);
        return state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && !GetCombatComp()->IsBlockPressed() &&
            !state->Info.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt);
    });
    CombatFSM.addLink(ECombatState::Attack, ECombatState::Idle, req);
    req.Unbind();
    req.BindLambda([this]() // manual interrupt
    {
        bool hasStanceAnims = Owner->AnimInstance->CurrentBlockAnim != nullptr;
        const auto state = CombatFSM.state(ECombatState::Attack);
        bool checkEnd = state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && GetCombatComp()->IsBlockPressed() &&
            !state->Info.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt);
        bool hasAnim = Owner->AnimInstance->CurrentBlockAnim != nullptr;
        if (!hasAnim)
             Owner->Logger.Log(GetCombatComp(), "Stance doesn't have block anim");
        return hasAnim && hasStanceAnims && IsBlockAllowed && (checkEnd || GetCombatComp()->CanBeInterruptedWithBlock(ECombatState::Attack));
    });
    CombatFSM.addLink(ECombatState::Attack, ECombatState::Block, req);
    req.Unbind();

    //=================================== Ability ====================================//
    CombatFSM.state(ECombatState::Ability)->Info.OnEnter.BindLambda([this]() { OnAbilityEnter(); });
    CombatFSM.state(ECombatState::Ability)->Info.OnLeave.BindLambda([this]() { OnAbilityLeave(); });
    req.BindLambda([this]() { return AbilityToIdle(); });
    CombatFSM.addLink(ECombatState::Ability, ECombatState::Idle, req);
}

bool UMeleeCombatStance::IsStanceValid(const ACharacterCreature* NewOwner) const
{
    auto mainWeapon = GetArmsFromSlot(EArmsSlot::MainHand);
    if (IsUsingDefaultWeapons && (NewOwner->EquipmentComponent->GetEquipmentFromSlot(EItemSlot::MainWeaponSlot).Item ||
        NewOwner->EquipmentComponent->GetEquipmentFromSlot(EItemSlot::OffMeaponSlot).Item))
        return false; // don't set unarmed stance if armed
    auto offWeapon = GetArmsFromSlot(EArmsSlot::OffHand); 
    auto shield = GetArmsFromSlot(EArmsSlot::Shield); 
    switch (StanceType)
    {
    case EWeaponCombination::TwoWeapon:
        return mainWeapon && AllowedWeaponTypes.Contains(mainWeapon->WeaponType) && 
            offWeapon && AllowedWeaponTypes.Contains(offWeapon->WeaponType);
    case EWeaponCombination::TwoHand:
    case EWeaponCombination::OneHand:
    {
        bool hasOffhandEquipped = IsUsingDefaultWeapons ? false : (NewOwner->EquipmentComponent->GetEquipmentFromSlot(EItemSlot::OffMeaponSlot).Item != nullptr);
        return mainWeapon && AllowedWeaponTypes.Contains(mainWeapon->WeaponType) && !offWeapon && !hasOffhandEquipped;
    }
    case EWeaponCombination::Shield:
        return mainWeapon && AllowedWeaponTypes.Contains(mainWeapon->WeaponType) && 
            shield && (shield->WeaponType == EWeaponType::Shield) && AllowedWeaponTypes.Contains(shield->WeaponType);
    default:
        return false;
    }
}

void URangeCombatStance::StanceSwitched(ACharacterCreature* NewOwner, const UCombatStance* DefaultStance)
{
    Super::StanceSwitched(NewOwner, DefaultStance);
    if (!CombatFSM.state(ECombatState::Attack)->Info.AttackInfo)
        CombatFSM.state(ECombatState::Attack)->Info.AttackInfo = &AttackInfo;
}

bool URangeCombatStance::IsStanceValid(const ACharacterCreature* NewOwner) const
{
    auto mainWeapon = GetArmsFromSlot(EArmsSlot::MainHand);
    auto offWeapon = GetArmsFromSlot(EArmsSlot::OffHand); 
    check(mainWeapon && !offWeapon);
    return StanceType == EWeaponCombination::Ranged && AllowedWeaponTypes.Contains(mainWeapon->WeaponType);
}

void URangeCombatStance::UpdateOwnerAnims(const UCombatStance* DefaultStance)
{
    Owner->AnimInstance->CurrentBlockedAttackAnim = BlockedAttackAnim ? BlockedAttackAnim : DefaultStance->BlockedAttackAnim;
    Owner->UpdateCCLOBAnims();

    Owner->AnimInstance->CurrentCrouchLocomotion = CrouchLocomotion ? CrouchLocomotion : DefaultStance->CrouchLocomotion;
    Owner->AnimInstance->CurrentStandLocomotion = StandLocomotion ? StandLocomotion : DefaultStance->StandLocomotion;;
    Owner->AnimInstance->CurrentAimOffset = AimOffset ? AimOffset : DefaultStance->AimOffset;;
    Owner->AnimInstance->CurrentBlockAnim = BlockAnim ? BlockAnim : DefaultStance->BlockAnim;;
    Owner->AnimInstance->CurrentDrawAnim = DrawAnim ? DrawAnim : DefaultStance->DrawAnim;;
    Owner->AnimInstance->CurrentSheathAnim = SheathAnim ? SheathAnim : DefaultStance->SheathAnim;;
}
