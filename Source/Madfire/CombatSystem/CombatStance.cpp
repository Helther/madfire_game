#include "CombatStance.h"
#include "Character/CharacterCreature.h"
#include "Items/RangedWeaponAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"


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
    Owner->AnimInstance->CurrentStandLocomotion = StandLocomotion ? StandLocomotion : DefaultStance->StandLocomotion;
    Owner->AnimInstance->CurrentAimOffset = AimOffset ? AimOffset : DefaultStance->AimOffset;
    Owner->AnimInstance->CurrentBlockAnim = BlockAnim ? BlockAnim : DefaultStance->BlockAnim;
    Owner->AnimInstance->CurrentParryAnim = ParryAnim ? ParryAnim : DefaultStance->ParryAnim;
    Owner->AnimInstance->CurrentDrawAnim = DrawAnim ? DrawAnim : DefaultStance->DrawAnim;
    Owner->AnimInstance->CurrentSheathAnim = SheathAnim ? SheathAnim : DefaultStance->SheathAnim;
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
    GetCombatComp()->ActivateAbility();
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
    Owner->ClearAbilityCameraMode();
}

bool UCombatStance::IdleToAbility() const
{ // check if not active and not in cooldown and more
    bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0;

    bool bIsCooldown = true;
    if (GetCombatComp()->CurrentAbility.IsValid())
        bIsCooldown = (!GetCombatComp()->CurrentAbility->bIsCooldown || GetCombatComp()->CurrentAbility->CurrentCharges > 0);
    return GetCombatComp()->IsAbilityPressed() && hasStamina && GetCombatComp()->CurrentAbility.IsValid() && bIsCooldown && !GetCombatComp()->IsOnGlobalCooldown();
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
            Owner->AnimInstance->CurrentSheathAnim : Owner->AnimInstance->CurrentDrawAnim, callBack,  Owner->AttributeComponent->GetAnimRateFromModifier(EAttributes::Dex));
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


URangeCombatStance::URangeCombatStance()
{
    CombatFSM.addState(ECombatState::Idle)
      .addState(ECombatState::WeaponSwitch)
      .addState(ECombatState::Attack)
      .addState(ECombatState::Block)
      .addState(ECombatState::Ability)
      .addState(ECombatState::Parry);
    CombatFSM.setCurrentState(ECombatState::Idle);
    //=================================== IDLE =====================================//
    CombatFSM.state(ECombatState::Idle)->Info.OnEnter.BindLambda([this]() {
        Owner->Logger.Log(GetCombatComp(),
                          *FString::Printf(TEXT("idle enter: %s"),
                                           GetCombatComp()->WeaponDrawn()
                                             ? TEXT("weapon on")
                                             : TEXT("weapon off")));
        auto &pendingStance = GetCombatComp()->PendingStance;
        if (pendingStance.IsSet())
            GetCombatComp()->SetNewStance(
              this, pendingStance.NewStance, pendingStance.DefaultStance);
    });
    CombatFSM.state(ECombatState::Idle)->Info.OnLeave.BindLambda([this]() {
        CombatFSM.state(ECombatState::Idle)->Info.ResetAnimInfo();
        Owner->Logger.Log(GetCombatComp(), "idle leave");
    });
    FLinkReq req;
    req.BindLambda([this]() {
        return CombatFSM.isInState(ECombatState::Idle)
          && (GetCombatComp()->IsWeaponSwitchPressed()
              || GetCombatComp()->PendingStance.IsSet());
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::WeaponSwitch, req);
    req.Unbind();
    req.BindLambda([this]() {
        bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0;
        bool hasAnim    = Owner->AnimInstance->CurrentBlockAnim != nullptr;
        if (!hasAnim)
            Owner->Logger.Log(GetCombatComp(), "Stance doesn't have block anim");
        return hasAnim && IsBlockAllowed && GetCombatComp()->WeaponDrawn()
          && CombatFSM.isInState(ECombatState::Idle) && GetCombatComp()->IsBlockPressed()
          && hasStamina;
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::Block, req);
    req.Unbind();
    req.BindLambda([this]() {
        bool hasAnim = AttackInfo.AttackMontage != nullptr;
        if (!hasAnim)
            Owner->Logger.Log(GetCombatComp(), "Stance doesn't have the attack info/anim");
        bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0;
        if (!GetCombatComp()->WeaponDrawn() || !GetCombatComp()->IsAttackPressed()
            || !hasStamina || Owner->GetMovementComponent()->IsTransitionInProgress() || !IsWeaponLoaded)
            return false;

        return true;
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::Attack, req);
    req.Unbind();
    req.BindLambda([this]() // check if not active and not in cooldown and more
        { return IdleToAbility(); });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::Ability, req);
    req.Unbind();

    //================================ WEAPON SWITCH ==============================//
    CombatFSM.state(ECombatState::WeaponSwitch)->Info.OnEnter.BindLambda([this]() {
        Owner->Logger.Log(GetCombatComp(), "weapon switch enter");
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(GetCombatComp(), &UCombatComponent::OnWeaponSwitchEnded);
        Owner->StartAnimation(GetCombatComp()->WeaponDrawn()
                                ? Owner->AnimInstance->CurrentSheathAnim
                                : Owner->AnimInstance->CurrentDrawAnim,
                              callBack,
                              Owner->AttributeComponent->GetAnimRateFromModifier(EAttributes::Dex));
    });
    CombatFSM.state(ECombatState::WeaponSwitch)->Info.OnLeave.BindLambda([this]() {
        Owner->Logger.Log(GetCombatComp(), "weapon switch leave");
        if (GetCombatComp()->WeaponDrawn()) // in combat mode
        {
            Owner->GetMovementComponent()->maxSpeedBlock = true;
            GetCombatComp()->CalculateMaxCombatSpeeds();
            Owner->GetMovementComponent()->bUseControllerDesiredRotation = true;
            Owner->GetMovementComponent()->bOrientRotationToMovement     = false;
            auto &pendingStance = GetCombatComp()->PendingStance;
            if (pendingStance.IsSet())
                pendingStance.Reset();
        }
        else // exit combat mode
        { // reset max speeds
            Owner->GetMovementComponent()->maxSpeedBlock = false;
            Owner->GetMovementComponent()->CalculateMaxSpeeds(Owner->GetMovementComponent()->GetSpeedModifier());
            Owner->GetMovementComponent()->bUseControllerDesiredRotation = false;
            Owner->GetMovementComponent()->bOrientRotationToMovement     = true;
        }
        CombatFSM.state(ECombatState::WeaponSwitch)->Info.ResetAnimInfo();
    });
    req.BindLambda([this]() // to idle after animation complete
        {
            return CombatFSM.state(ECombatState::WeaponSwitch)->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded);
        });
    CombatFSM.addLink(ECombatState::WeaponSwitch, ECombatState::Idle, req);
    req.Unbind();

    //=================================== Block =====================================//
    CombatFSM.state(ECombatState::Block)->Info.OnEnter.BindLambda([this]() {
        Owner->Logger.Log(GetCombatComp(), "block enter");
        GetCombatComp()->CalculateMaxBlockingSpeed(
          Owner->GetMovementComponent()->GetSpeedModifier());
        Owner->AttributeComponent->Stamina->RegenCoef +=
          GetCombatComp()->BlockStaminaRegenRateMod; // slowing down stamina regen rate
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(GetCombatComp(), &UCombatComponent::OnBlockEnded);
        Owner->StartAnimation(Owner->AnimInstance->CurrentBlockAnim,
                              callBack,
                              GetCombatComp()->AttackSpeed,
                              FAnimSectionNames::Start);
    });
    CombatFSM.state(ECombatState::Block)->Info.OnLeave.BindLambda([this]() {
        Owner->Logger.Log(GetCombatComp(), "block leave");
        GetCombatComp()->CalculateMaxCombatSpeeds();
        Owner->AttributeComponent->Stamina->RegenCoef -=
          GetCombatComp()->BlockStaminaRegenRateMod; // reseting stamina regen rate
        CombatFSM.state(ECombatState::Block)->Info.ResetAnimInfo();
    });
    req.BindLambda([this]() // to idle after animation complete
    {
        const auto state = CombatFSM.state(ECombatState::Block);
        return state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded)
        && !state->Info.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt);
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
        return hasAnim && isNotFalling && state->Info.CheckAnimInfo(ECombatStateAnimInfo::BlockOrParryActive) &&  GetCombatComp()->IsAttackPressed() && hasStamina;
    });
    CombatFSM.addLink(ECombatState::Block, ECombatState::Parry, req);
    req.Unbind();
    //=================================== Ability ====================================//
    CombatFSM.state(ECombatState::Ability)->Info.OnEnter.BindLambda([this]()
    {
        OnAbilityEnter();
    });
    CombatFSM.state(ECombatState::Ability)->Info.OnLeave.BindLambda([this]()
    {
        OnAbilityLeave();
    });
    req.BindLambda([this]() { return AbilityToIdle(); });
    CombatFSM.addLink(ECombatState::Ability, ECombatState::Idle, req);
    req.Unbind();

    //=================================== Parry =====================================//
    // Parry will take role of a melee strike
    CombatFSM.state(ECombatState::Parry)->Info.OnEnter.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), "parry enter");
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(GetCombatComp(), &UCombatComponent::OnParryEnded);
        Owner->StartAnimation(Owner->AnimInstance->CurrentParryAnim, callBack,  Owner->AttributeComponent->GetAnimRateFromModifier(EAttributes::Dex));
        Owner->AttributeComponent->Stamina->Consume(MeleeAttackStaminaCost);
        Owner->AttributeComponent->Stamina->BlockRegen();
        Owner->GetMovementComponent()->RotationRate.Yaw = GetCombatComp()->ActionRotationRate;
    });
    CombatFSM.state(ECombatState::Parry)->Info.OnLeave.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), *FString::Printf(TEXT("parry leave")));
        GetCombatComp()->ResetActionState(ECombatState::Parry);
        GetCombatComp()->CharactersHit.Reset();
        auto activeWeapons = GetCombatComp()->FindActiveMeleeWeapons();
        for (auto& weaponActor : activeWeapons)
            UCombatComponent::SetWeaponCollision(weaponActor, false);
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
}

bool URangeCombatStance::IsStanceValid(const ACharacterCreature *NewOwner) const
{
    auto mainWeapon = GetArmsFromSlot(EArmsSlot::MainHand);
    auto offWeapon = GetArmsFromSlot(EArmsSlot::OffHand); 
    
    return !offWeapon && mainWeapon && StanceType == EWeaponCombination::Ranged && AllowedWeaponTypes.Contains(mainWeapon->WeaponType);
}

void URangeCombatStance::UpdateOwnerAnims(const UCombatStance* DefaultStance)
{
    Owner->AnimInstance->CurrentBlockedAttackAnim = BlockedAttackAnim ? BlockedAttackAnim : DefaultStance->BlockedAttackAnim;
    Owner->UpdateCCLOBAnims();

    Owner->AnimInstance->CurrentCrouchLocomotion = CrouchLocomotion ? CrouchLocomotion : DefaultStance->CrouchLocomotion;
    Owner->AnimInstance->CurrentStandLocomotion = StandLocomotion ? StandLocomotion : DefaultStance->StandLocomotion;
    Owner->AnimInstance->CurrentAimOffset = AimOffset ? AimOffset : DefaultStance->AimOffset;
    Owner->AnimInstance->CurrentBlockAnim = BlockAnim ? BlockAnim : DefaultStance->BlockAnim;
    Owner->AnimInstance->CurrentParryAnim = ParryAnim ? ParryAnim : DefaultStance->ParryAnim;
    Owner->AnimInstance->CurrentDrawAnim = DrawAnim ? DrawAnim : DefaultStance->DrawAnim;
    Owner->AnimInstance->CurrentSheathAnim = SheathAnim ? SheathAnim : DefaultStance->SheathAnim;
}

float URangeCombatStance::CalculateAmmoInitialSpeed(UItemWeaponRanged* Weapon) const
{
    return Weapon->MaxTensionStrength * Weapon->TensionCoefficient;
}

void URangeCombatStance::StanceSwitched(ACharacterCreature* NewOwner, const UCombatStance* DefaultStance)
{
    Super::StanceSwitched(NewOwner, DefaultStance);
    if (!CombatFSM.state(ECombatState::Attack)->Info.AttackInfo)
        CombatFSM.state(ECombatState::Attack)->Info.AttackInfo = &AttackInfo;
}

void URangeCombatStance::ResetStance()
{
    CleanUpAmmoActor();
    Super::ResetStance();
}

void URangeCombatStance::FireLoadedAmmo()
{
    check(LoadedAmmo);
    IsWeaponLoaded = false;

    FVector CameraLocation;
    FRotator CameraRotation;
    AController* Controller = Owner->GetController();
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (PC != nullptr)
        PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
    else
    {
        CameraLocation = Owner->GetActorLocation();
        CameraRotation = Controller->GetControlRotation();
    }

    FVector MuzzleDirection = CameraRotation.Vector().GetSafeNormal();
    FVector LaunchDirection = MuzzleDirection;
    FVector AmmoLocation = LoadedAmmo->GetActorLocation();
    float MaxFocalDistance = 2048.f;  // TODO devise or calculate
    float MinFocalDistance = 128.f;
    
    FTraceParams traceParams;
    traceParams.isDebug = false;
    traceParams.ActorsToIgnore.Add(Owner);
    // TODO trace endpoint location unfortunately depends of camera zoom distance
    FTraceSpawnParams traceSpawnParams;
    traceSpawnParams.Start = CameraLocation;
    traceSpawnParams.End = CameraLocation + (MuzzleDirection * MaxFocalDistance);

    FHitResult hit(ForceInit);
    // Calculate the direction vector, between two points
    if(UTraceManager::LineTraceSingleByChannel(GetWorld(), traceParams, traceSpawnParams, hit, ECollisionChannel::ECC_Visibility))
        LaunchDirection = hit.ImpactPoint - AmmoLocation;
    else
        LaunchDirection = traceSpawnParams.End - AmmoLocation;
    if (LaunchDirection.Length() < MinFocalDistance)  // if hit was too close to the pawn use MinFocalDistance
        LaunchDirection = CameraLocation + (MuzzleDirection * MaxFocalDistance) - AmmoLocation;
    LaunchDirection.Normalize();
    LaunchDirection = LaunchDirection.Rotation().Vector();

    AItemActor* weaponActor = Owner->CombatComponent->GetCurrentCombatStance()->GetArmsActorFromSlot(EArmsSlot::MainHand);
    auto rangedWeapon = Cast<UItemWeaponRanged>(weaponActor->ItemBase);
    float LaunchInitialSpeed = CalculateAmmoInitialSpeed(rangedWeapon);
    LoadedAmmo->StackCount = 1;
    LoadedAmmo->LaunchProjectile(LaunchDirection, LaunchInitialSpeed, rangedWeapon->AmmoDamageCoefficient);
    LoadedAmmo = nullptr;
    // TODO LoadAmmoCount > 1 don't deload, spawn another ammo actor instead
    if (auto actorMesh = Cast<USkeletalMeshComponent>(weaponActor->ItemMesh))
        if (auto animInstance = Cast<URangedWeaponAnimInstance>(actorMesh->GetAnimInstance()))
            animInstance->IsFired = true;
}

void URangeCombatStance::SpawnAttachAmmoActor()
{
    auto baseActor = Owner->EquipmentComponent->GetEquipmentFromSlot(EItemSlot::AmmoSlot).Item;
    auto baseItem = baseActor->ItemBase;
    auto ammoItem = Cast<UItemAmmo>(baseItem);
    auto newAmmoActor = GetWorld()->SpawnActor<AItemActorProjectile>(ammoItem->ProjectileActor);
    auto newAmmoItem = DuplicateObject(baseItem, newAmmoActor);

    ACharacterBase::SetupItemActorCollision(newAmmoActor, false);
    newAmmoActor->SetOwner(Owner);
    newAmmoActor->ItemBase = newAmmoItem;
    newAmmoActor->StackCount = LoadAmmoCount;
    if (baseActor->StackCount > LoadAmmoCount)
        baseActor->StackCount -= LoadAmmoCount;
    else
        Owner->EquipmentComponent->Unequip(EItemSlot::AmmoSlot);

    // attach to character left grip hand slot
    newAmmoActor->AttachToComponent(Owner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FEquipmentSlotNames::WeaponOff);
    LoadedAmmo = newAmmoActor;
}

void URangeCombatStance::AttachAmmoActorToWeapon()
{
    check(LoadedAmmo);
    AItemActor* weaponActor = Owner->CombatComponent->GetCurrentCombatStance()->GetArmsActorFromSlot(EArmsSlot::MainHand);
    check(weaponActor);
    LoadedAmmo->AttachToComponent(weaponActor->ItemMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FItemActorEquipmentSlotNames::WeaponLoadedAmmo);
}

void URangeCombatStance::CleanUpAmmoActor()
{
    IsWeaponLoaded = false;
    if (LoadedAmmo)
    {
        auto ammoItem = LoadedAmmo->ItemBase;
        Owner->InventoryComponent->AddToInventory(ammoItem, LoadAmmoCount); // return loaded ammo to the inventory TODO until we think of something better
        LoadedAmmo->Destroy();
        LoadedAmmo = nullptr;
    }
}

void URangeCombatStance::OnReloadEnded(UAnimMontage* AnimMontage, bool bInterrupted)
{
    auto& stateInfo = CombatFSM.state(ECombatState::Reload)->Info;
    if (!bInterrupted || stateInfo.CheckAnimInfo(ECombatStateAnimInfo::AmmoGrabbed))
    {
        if (!stateInfo.CheckAnimInfo(ECombatStateAnimInfo::AmmoAttach))
            AttachAmmoActorToWeapon();
        IsWeaponLoaded = true;
    }
    else
    {
        AItemActor* weaponActor = Owner->CombatComponent->GetCurrentCombatStance()->GetArmsActorFromSlot(EArmsSlot::MainHand);
        if (auto actorMesh = Cast<USkeletalMeshComponent>(weaponActor->ItemMesh))
            if (auto animInstance = Cast<URangedWeaponAnimInstance>(actorMesh->GetAnimInstance()))
            {
                animInstance->IsLoading = false;
                animInstance->IsLoaded = false;
            }
    }

    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
    GetCombatComp()->UpdateCombatFSM();
}

EAttributes URangeCombatStance::GetAnimRateAttribute() const
{
    return EAttributes::Dex;
}


UBowCombatStance::UBowCombatStance() : Super()
{
    //=================================== Attack ====================================//
    CombatFSM.state(ECombatState::Attack)->Info.OnEnter.BindLambda([this]() {
        Owner->Logger.Log(GetCombatComp(), "attack enter");
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(GetCombatComp(), &UCombatComponent::OnAttackEnded);
        // TODO calculate speed from anim length and bow max tension curve time
        // TODO call UAnimInstance::Montage_Play to play anim backward when stop pulling
        EAttributes modAttribute = EAttributes::Dex;
        if (AItemActor* weaponActor = Owner->CombatComponent->GetCurrentCombatStance()->GetArmsActorFromSlot(EArmsSlot::MainHand))
        {
            if (auto weaponItem = Cast<UItemArms>(weaponActor->ItemBase))
                if (weaponItem->WeaponType == EWeaponType::Longbow)
                modAttribute = EAttributes::Str;
        }
        Owner->StartAnimation(AttackInfo.AttackMontage,
                              callBack,
                              Owner->AttributeComponent->GetAnimRateFromModifier(modAttribute),
                              AttackInfo.AnimSectionName);
        CombatFSM.state(ECombatState::Attack)
          ->Info.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, true);
        Owner->AttributeComponent->Stamina->Consume(AttackInfo.StaminaCost);
        Owner->AttributeComponent->Stamina->BlockRegen();
        GetCombatComp()->CalculateMaxBlockingSpeed(
          Owner->GetMovementComponent()->GetSpeedModifier());
    });
    CombatFSM.state(ECombatState::Attack)->Info.OnLeave.BindLambda([this]() {
        Owner->Logger.Log(GetCombatComp(), "attack leave");
        Owner->AttributeComponent->Stamina->UnblockRegen();
        CombatFSM.state(ECombatState::Attack)->Info.ResetAnimInfo();
        CombatFSM.state(ECombatState::Attack)->Info.ChargeTime = 0.f;
        GetCombatComp()->CalculateMaxCombatSpeeds();
    });
    FLinkReq req;
    req.BindLambda([this]() {
        const auto state = CombatFSM.state(ECombatState::Attack);
        return state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded)
          && !GetCombatComp()->IsBlockPressed();
    });
    CombatFSM.addLink(ECombatState::Attack, ECombatState::Idle, req);
    req.Unbind();
    req.BindLambda(
      [this]() // manual interrupt
      {
          bool hasStanceAnims = Owner->AnimInstance->CurrentBlockAnim != nullptr;
          const auto state    = CombatFSM.state(ECombatState::Attack);
          bool checkEnd       = state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded)
            && GetCombatComp()->IsBlockPressed()
            && !state->Info.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt);
          bool hasAnim = Owner->AnimInstance->CurrentBlockAnim != nullptr;
          if (!hasAnim)
              Owner->Logger.Log(GetCombatComp(), "Stance doesn't have block anim");
          return hasAnim && hasStanceAnims && IsBlockAllowed
            && (checkEnd || GetCombatComp()->CanBeInterruptedWithBlock(ECombatState::Attack));
      });
    CombatFSM.addLink(ECombatState::Attack, ECombatState::Block, req);
    req.Unbind();
    // req.BindLambda(); // TODO define cond for case when start drawing right after shooting from ECombatState::Attack
    // CombatFSM.addLink(ECombatState::Attack, ECombatState::Reload, req);

    //=================================== Reload ====================================//
    CombatFSM.addState(ECombatState::Reload);
    CombatFSM.state(ECombatState::Reload)->Info.OnEnter.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), "reload enter");
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(this, &URangeCombatStance::OnReloadEnded);
        Owner->StartAnimation(ReloadAnim, callBack, Owner->AttributeComponent->GetAnimRateFromModifier(EAttributes::Dex));
    });
    CombatFSM.state(ECombatState::Reload)->Info.OnLeave.BindLambda([this]()
    {
        CombatFSM.state(ECombatState::Reload)->Info.ResetAnimInfo();
        Owner->Logger.Log(GetCombatComp(), "reload leave");
    });
    req.BindLambda([this] ()
    {
        check(ReloadAnim && "no reload anim");
        bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0;
        auto AmmoItem = Owner->EquipmentComponent->GetEquipmentFromSlot(EItemSlot::AmmoSlot).Item;
        bool hasAmmo = AmmoItem && AmmoItem->StackCount;
        
        return ReloadAnim && IsBlockAllowed && !IsWeaponLoaded && hasStamina && hasAmmo
            && CombatFSM.isInState(ECombatState::Idle) && GetCombatComp()->IsAttackPressed()
            && GetCombatComp()->WeaponDrawn();
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::Reload, req);
    req.Unbind();
    req.BindLambda([this] ()
    {
        return CombatFSM.state(ECombatState::Reload)->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) &&
            !GetCombatComp()->IsAttackPressed();
    });
    CombatFSM.addLink(ECombatState::Reload, ECombatState::Idle, req);
    req.Unbind();
    req.BindLambda([this]()
    {
        bool hasAnim = AttackInfo.AttackMontage != nullptr;
        if (!hasAnim)
            Owner->Logger.Log(GetCombatComp(), "Stance doesn't have the attack info/anim");
        auto AmmoItem = Owner->EquipmentComponent->GetEquipmentFromSlot(EItemSlot::AmmoSlot).Item;
        bool hasAmmo = AmmoItem && AmmoItem->StackCount;
        bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0;
        if (!GetCombatComp()->WeaponDrawn() || !GetCombatComp()->IsAttackPressed()
            || !hasStamina || Owner->GetMovementComponent()->IsTransitionInProgress() || !hasAmmo)
            return false;

        return true;
    });
    CombatFSM.addLink(ECombatState::Reload, ECombatState::Attack, req);
}

UCrossbowCombatStance::UCrossbowCombatStance() : Super()
{
    CombatFSM.addState(ECombatState::Reload);

    //=================================== Attack ====================================//
    CombatFSM.state(ECombatState::Attack)->Info.OnEnter.BindLambda([this]() {
        Owner->Logger.Log(GetCombatComp(), "attack enter");
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(GetCombatComp(), &UCombatComponent::OnAttackEnded);
        Owner->StartAnimation(
          AttackInfo.AttackMontage, callBack, 1.f, AttackInfo.AnimSectionName);
        Owner->AttributeComponent->Stamina->Consume(AttackInfo.StaminaCost);
        Owner->AttributeComponent->Stamina->BlockRegen();
        GetCombatComp()->CalculateMaxBlockingSpeed(
          Owner->GetMovementComponent()->GetSpeedModifier());
    });
    CombatFSM.state(ECombatState::Attack)->Info.OnLeave.BindLambda([this]() {
        Owner->Logger.Log(GetCombatComp(), "attack leave");
        Owner->AttributeComponent->Stamina->UnblockRegen();
        CombatFSM.state(ECombatState::Reload)->Info.ResetAnimInfo();
        GetCombatComp()->CalculateMaxCombatSpeeds();
    });
    FLinkReq req;
    req.BindLambda([this]()
    {
        const auto state = CombatFSM.state(ECombatState::Attack);
        return state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded);
    });
    CombatFSM.addLink(ECombatState::Attack, ECombatState::Idle, req);
    req.Unbind();
    //=================================== Reload ====================================//
    CombatFSM.state(ECombatState::Reload)->Info.OnEnter.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), "reload enter");
        Owner->AttributeComponent->Stamina->Consume(ReloadStaminaCost);
        Owner->AttributeComponent->Stamina->BlockRegen();
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(this, &URangeCombatStance::OnReloadEnded);
        Owner->StartAnimation(ReloadAnim, callBack, Owner->AttributeComponent->GetAnimRateFromModifier(EAttributes::Str));
        AItemActor* weaponActor = Owner->CombatComponent->GetCurrentCombatStance()->GetArmsActorFromSlot(EArmsSlot::MainHand);
        if (auto actorMesh = Cast<USkeletalMeshComponent>(weaponActor->ItemMesh))
            if (auto animInstance = Cast<URangedWeaponAnimInstance>(actorMesh->GetAnimInstance()))
            {
                animInstance->IsLoading = true;
                animInstance->LoadingPlayRate = Owner->AttributeComponent->GetAnimRateFromModifier(GetAnimRateAttribute());
            }
    });
    CombatFSM.state(ECombatState::Reload)->Info.OnLeave.BindLambda([this]()
    {
        Owner->Logger.Log(GetCombatComp(), "reload leave");
        Owner->AttributeComponent->Stamina->UnblockRegen();
    });
    req.BindLambda([this] ()
    {
        check(ReloadAnim && "no reload anim");
        bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0;
        auto AmmoItem = Owner->EquipmentComponent->GetEquipmentFromSlot(EItemSlot::AmmoSlot).Item;
        bool hasAmmo = AmmoItem && AmmoItem->StackCount;

        return ReloadAnim && IsBlockAllowed && !IsWeaponLoaded && hasStamina && hasAmmo
            && CombatFSM.isInState(ECombatState::Idle) && GetCombatComp()->IsAttackPressed()
            && GetCombatComp()->WeaponDrawn();
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::Reload, req);
    req.Unbind();
    req.BindLambda([this] ()
    {
        return CombatFSM.state(ECombatState::Reload)->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) &&
            !GetCombatComp()->IsAttackPressed();
    });
    CombatFSM.addLink(ECombatState::Reload, ECombatState::Idle, req);
}

EAttributes UCrossbowCombatStance::GetAnimRateAttribute() const
{
    return EAttributes::Str;
}
