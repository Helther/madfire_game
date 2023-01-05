#include "CombatComponent.h"

#include "../CharacterCreature.h"
#include "Kismet/KismetMathLibrary.h"


UCombatComponent::UCombatComponent(const class FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    // TODO temp test
    AttackSpeed = 1.f;// TODO set in update from dex mod?
    UnarmedBaseDamage = 1.f;
    CastSpeed = 1.f;
    BlockStaminaRegenRateMod = 0.5f;
    CombatModeRotationRate = 500.f;
    ActionRotationRate = 100.f;
    ChargeStartTime = 0.3f;
    BlockStaminaCost = 10.f;
    KnockdownThreshhold = 1.5f;
    ChamberThreshhold = 1.3f;
    MaxFacingAngle = 45.f;
    ShieldWeightDamageMod = 0.5f;
    // end temp

    AbilityPanel.SetNum(10);
}

FName UCombatComponent::GetSlotFromWeaponType(EWeaponType type, bool draw, bool mainHand)
{
    if (draw)
    {
        if (type == EWeaponType::Shield)
            return FEquipmentSlotNames::Shield;
        return mainHand ? FEquipmentSlotNames::WeaponMain : FEquipmentSlotNames::WeaponOff;
    }
    else
    {
        // TODO consider adding special sheath anims for two handed weapons
        switch (type)
        {
        case EWeaponType::Quaterstaff:
        case EWeaponType::Spear:
        case EWeaponType::Crossbow:
        case EWeaponType::Battleaxe:
        case EWeaponType::Warhammer:
        case EWeaponType::Greatclub:
        case EWeaponType::Halberd:
        case EWeaponType::Longbow:
        case EWeaponType::Shortbow:
            return FEquipmentSlotNames::SheathWeaponBack;
        case EWeaponType::Shield:
            return FEquipmentSlotNames::SheathShieldBack;
        case EWeaponType::Unarmed:
            return mainHand ? FEquipmentSlotNames::WeaponMain : FEquipmentSlotNames::WeaponOff;
        default:
            return mainHand ? FEquipmentSlotNames::SheathMain : FEquipmentSlotNames::SheathOff;
        }
    }
}

float UCombatComponent::CalculateWeaponDamage(const UItemArms* weapon, float attackInfoDamageMod, float strMod)
{
    switch (weapon->WeaponType)
    {
    case EWeaponType::Unarmed:
    {
        auto gloves = Owner->EquipmentComponent->GetEquipmentFromSlot(EItemSlot::HandsSlot).Item;
        if (gloves)
            return FMath::Max((UnarmedBaseDamage + gloves->ItemBase->Weight) + strMod, UnarmedBaseDamage);
        else
            return FMath::Max(UnarmedBaseDamage + strMod, UnarmedBaseDamage);
    }
    case EWeaponType::Shield:
        return FMath::Max(UnarmedBaseDamage, UnarmedBaseDamage + strMod + (weapon->Weight * ShieldWeightDamageMod));
    // TODO range weapons
    default:
    {
        auto theWeapon = Cast<UItemWeapon>(weapon);
        check(theWeapon && "CalculateWeaponDamage: invalid weapon class");
        return FMath::Max(theWeapon->DamageValue * attackInfoDamageMod + strMod, UnarmedBaseDamage);
    }
    }
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    Owner = Cast<ACharacterCreature>(GetOwner());
    check(Owner != nullptr && "UCombatComponent: No pawn");
    // set default stance as current and setup weapon actors
    ResolveNewStance();
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    auto currentState = CurrentCombatStance ? GetCombatFSM().currentState() : nullptr;
    if (currentState && CurrentCombatStance && currentState->Id == ECombatState::Attack && AttackPressed)// check if charging
    {
        if (currentState->Info.CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed))
        {
            currentState->Info.ChargeTime += DeltaTime;
            // check if held attack for at least some time to start charging and has charge stage
            if (currentState->Info.ChargeStage == -1 && currentState->Info.ChargeTime < ChargeStartTime)
                return;
            if (CurrentCombatStance->IsMelee())
            {
                auto meleeStance = Cast<UMeleeCombatStance>(CurrentCombatStance);
                auto& currentCombo = meleeStance->ComboAttacks[meleeStance->GetComboCount()];
                if (currentCombo.ChargeAttackStages.Num() == 0)
                    return;
                // charged enough to go to next stage if has any
                if (currentState->Info.ChargeStage == -1 || ((currentState->Info.ChargeStage + 1 < currentCombo.ChargeAttackStages.Num()) && currentState->Info.ChargeTime >=
                    currentCombo.ChargeAttackStages[currentState->Info.ChargeStage + 1].ChargeTimeNeeded))
                {
                    if (currentCombo.ChargeAttackStages[currentState->Info.ChargeStage + 1].Info.AttackMontage != nullptr)
                    {
                        ++currentState->Info.ChargeStage;
                        currentState->Info.ResetAnimInfo();
                        currentState->Info.AttackInfo = &currentCombo.ChargeAttackStages[currentState->Info.ChargeStage].Info;
                        currentState->Info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                        currentState->Info.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, true);
                        FOnMontageBlendingOutStarted callBack;
                        callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
                        Owner->StartAnimation(currentState->Info.AttackInfo->AttackMontage, callBack, AttackSpeed, currentState->Info.AttackInfo->AnimSectionName);
                    }
                }
            }
        }
    }
}

bool UCombatComponent::UpdateCombatFSM()
{
    int oldState = GetCombatFSM().currentState()->Id;
    auto newState = GetCombatFSM().proccessNextState();
    if (newState != nullptr)
    {
        GetCombatFSM().setCurrentState(newState->Id);
        OnCombatStateChanged.Broadcast(oldState,newState->Id);
    }
    return newState != nullptr;
}

void UCombatComponent::ResetActionState(int StateId)
{
    // reset root motion ignore
    Owner->AnimInstance->OnMovementAllowed(true);
    Owner->AnimInstance->OnRotationAllowed(false);
    Owner->AttributeComponent->Stamina->UnblockRegen();
    GetCombatFSM().state(StateId)->Info.ResetAnimInfo();
    Owner->GetMovementComponent()->RotationRate.Yaw = CombatModeRotationRate;
}

void UCombatComponent::OnMovementModeChanged(int oldMode, int newMode)
{ 
    if (CurrentCombatStance && GetCombatFSM().currentState() && GetCombatFSM().isInState(ECombatState::Attack))
    {
        // check if started falling during regular attack
        if (newMode == EMovementMode::MOVE_Falling && !GetCombatFSM().currentState()->Info.CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed))
            Owner->InterruptAnimation(ECCState::NoCC);
        // check if landed during air attack to finish anim
        if (newMode == EMovementMode::MOVE_Walking && GetCombatFSM().currentState()->Info.AttackInfo->Special == ESpecialType::Air)
        {
            GetCombatFSM().currentState()->Info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
            FOnMontageBlendingOutStarted callBack;
            callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
            Owner->StartAnimation(GetCombatFSM().currentState()->Info.AttackInfo->AttackMontage, callBack, AttackSpeed, FAnimSectionNames::End);	
        }
    }
}

void UCombatComponent::OnWeaponOverlap(UPrimitiveComponent* comp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    auto hitActor = SweepResult.GetActor();
    if (hitActor == nullptr || OtherActor == Owner || HitOwnMeleeWeapons(comp, OtherActor))// Check self and weapon
        return;
    if (GetCombatFSM().isInState(ECombatState::Attack))
    {
        const auto &info = GetCombatFSM().state(ECombatState::Attack)->Info;
        auto activeWeapon = FindHitMeleeWeapon(comp);
        check(activeWeapon && "Should have a weapon");
        if (info.CheckAnimInfo(ECombatStateAnimInfo::CollisionOn) && !HasAlreadyHitTarget(hitActor, activeWeapon))
        {
            CharactersHit.Emplace(TPair<AActor*, UItemArms*>(hitActor, activeWeapon));
            auto hitCharacter = Cast<ACharacterBase>(hitActor);
            if (hitCharacter)
            {
                if (hitCharacter->Cheats.InvulnerableToHit)
                {
                    Owner->Logger.Log(this, *FString::Printf(TEXT("OnBeginOverlap: invul character!")));
                    return;
                }
                // check for chamber scenario
                auto hitCombatCharacter = Cast<ACharacterCreature>(hitActor);
                auto enemyActiveWeapon = FindCharacterActiveWeapon(hitCombatCharacter);
                if (hitCombatCharacter && IsChamberValid(hitCombatCharacter, enemyActiveWeapon, activeWeapon))
                {
                    EChamberRes res = ResolveChamber(hitCombatCharacter,enemyActiveWeapon, activeWeapon);
                    if (res == EChamberRes::ChamberState)
                        return;
                }
                Owner->Logger.Log(this, "OnBeginOverlap: Hit character!");
                int32 strMod = UAttributeSetHelper::GetModifier(hitCharacter->AttributeComponent->Attributes, EAttributes::Str);
                const float damage = CalculateWeaponDamage(activeWeapon, GetCombatFSM().state(ECombatState::Attack)->Info.AttackInfo->DamageMod, strMod);
                float force = CalculateHitForce(activeWeapon->Weight, strMod, info.AttackInfo->PowerMod); // Use Force, Luke
                // check what body part was hit
                // TODO apply sound and particle effect based on material surface type
                if (OtherComp)
                {
                    FComponentQueryParams params;
                    params.bTraceComplex = true;
                    params.bReturnPhysicalMaterial = true;
                    params.AddIgnoredActor(Owner);
                    // Avatar->GetAttachedActors(/*out*/ AttachedActors);
                    // TraceParams.AddIgnoredActors(AttachedActors); TODO add character child actors
                    TArray<FHitResult> hits;
                    auto weaponCollision = Cast<UBoxComponent>(comp);
                    FVector collisionVertExtent = weaponCollision->GetComponentTransform().GetRotation().GetUpVector() * weaponCollision->GetScaledBoxExtent().Z;
                    FVector startTrace = weaponCollision->GetComponentTransform().GetLocation() - collisionVertExtent;
                    FVector endTrace = weaponCollision->GetComponentTransform().GetLocation() + collisionVertExtent;
                    GetWorld()->ComponentSweepMulti(hits, weaponCollision, startTrace, endTrace, weaponCollision->GetComponentQuat(), params);
                    auto Pred = [OtherComp](const FHitResult& Other)
                    {
                        return Other.GetComponent() == OtherComp;
                    };
                    FHitResult* hit = hits.FindByPredicate(Pred);
                    if (hit)
                    {
                        Owner->Logger.Log(this, hit->BoneName.ToString());
                        if (hit->PhysMaterial.Get())
                            Owner->Logger.Log(this, *FString::Printf(TEXT("has material: %s"), *hit->PhysMaterial->GetName()));
                        else
                            Owner->Logger.Log(this, "no material");
                    }
                }
                hitCharacter->TakeDamage(damage, FForceDamageEvent(activeWeapon->DamageType, force), Owner->GetController(), Owner);
            }
            else // hit an obsticle
            {
                if (SweepResult.GetComponent()->GetCollisionObjectType() == ECollisionChannel::ECC_WorldStatic) // check if hit a wall and interrupt with loss of balance
                {
                    Owner->Logger.Log(this, "OnBeginOverlap Hit wall!");
                    Owner->InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::HitWall);
                }
                else // try to damage the actor
                {
                    Owner->Logger.Log(this, "OnBeginOverlap Hit actor!");
                    const float damage = CalculateWeaponDamage(activeWeapon, GetCombatFSM().state(ECombatState::Attack)->Info.AttackInfo->DamageMod, 0);
                    hitActor->TakeDamage(damage, FDamageEvent(activeWeapon->DamageType), Owner->GetController(), Owner);
                }
            }
            if (info.AttackInfo && info.AttackInfo->CustomOnHitEvent.IsBound())
                info.AttackInfo->CustomOnHitEvent.Broadcast(Owner, hitActor);
        }
    }
}

bool UCombatComponent::IsInActionState() const
{
    return GetCombatFSM().isInState(ECombatState::Attack) || GetCombatFSM().currentState()->Id == ECombatState::Parry;
}

bool UCombatComponent::IsChamberValid(const ACharacterCreature* HitCharacter, const UItemArms* HitActorActiveWeapon, const UItemArms* OwnerActiveWeapon) const
{
    if (HitCharacter->CombatComponent->GetCombatFSM().isInState(ECombatState::Attack))
    {
        auto& enemyAttackInfo = HitCharacter->CombatComponent->GetCombatFSM().state(ECombatState::Attack)->Info;
        auto& attackInfo = Owner->CombatComponent->GetCombatFSM().state(ECombatState::Attack)->Info;
        bool shouldResolveChamber = !enemyAttackInfo.CheckAnimInfo(ECombatStateAnimInfo::CheckedForChamber) ||
            !attackInfo.CheckAnimInfo(ECombatStateAnimInfo::CheckedForChamber);
        if (shouldResolveChamber && enemyAttackInfo.CheckAnimInfo(ECombatStateAnimInfo::CollisionOn) && GetAngleBetweenActors(Owner, HitCharacter) <= MaxFacingAngle)
        {
            check(OwnerActiveWeapon && HitActorActiveWeapon && "Characters should have weapons set");
            // check if both characters have simililar weapon types
            return (HitActorActiveWeapon->WeaponType != EWeaponType::Unarmed) ? OwnerActiveWeapon->WeaponType != EWeaponType::Unarmed :
                OwnerActiveWeapon->WeaponType == EWeaponType::Unarmed;
        }
    }

    return false;
}

UCombatComponent::EChamberRes UCombatComponent::ResolveChamber(ACharacterCreature* HitCharacter, const UItemArms* HitActorActiveWeapon, const UItemArms* OwnerActiveWeapon)
{
    Owner->Logger.Log(this, "Chamber: enter!");
    // calculate characters attack powers
    int32 strMod = UAttributeSetHelper::GetModifier(Owner->AttributeComponent->Attributes, EAttributes::Str);
    auto& characterAttackInfo = Owner->CombatComponent->GetCombatFSM().state(ECombatState::Attack)->Info;
    float force = CalculateHitForce(OwnerActiveWeapon->Weight, strMod, characterAttackInfo.AttackInfo->PowerMod);
    int32 enemyStrMod = UAttributeSetHelper::GetModifier(HitCharacter->AttributeComponent->Attributes, EAttributes::Str);
    auto& enemyAttackInfo = HitCharacter->CombatComponent->GetCombatFSM().state(ECombatState::Attack)->Info;
    float enemyForce = CalculateHitForce(HitActorActiveWeapon->Weight, enemyStrMod, enemyAttackInfo.AttackInfo->PowerMod);
    bool enemyStronger = enemyForce > force;
    float forceRatio = enemyStronger ? enemyForce/force : force/enemyForce;

    characterAttackInfo.ChangeAnimInfo(ECombatStateAnimInfo::CheckedForChamber, true);
    enemyAttackInfo.ChangeAnimInfo(ECombatStateAnimInfo::CheckedForChamber, true);
    bool characterIsInterruptable = !characterAttackInfo.AttackInfo->IsNotInterruptable;
    bool enemyIsInterruptable = !enemyAttackInfo.AttackInfo->IsNotInterruptable;
    EChamberRes state;

    if (forceRatio > ChamberThreshhold)
    { // one is stronger than other
        // apply reaction to weaker one
        auto staggeredCharacter = enemyStronger ? (characterIsInterruptable ? Owner : nullptr) : (enemyIsInterruptable ? HitCharacter : nullptr);
        if (staggeredCharacter)
        {
            staggeredCharacter->InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::None);
            staggeredCharacter->Logger.Log(this, "Chamber: got staggered!");
        }
        // apply reaction to stronger one
        auto strongerCharacter = enemyStronger ? (enemyIsInterruptable ? HitCharacter : nullptr) : (characterIsInterruptable ? Owner : nullptr);
        if (forceRatio <= KnockdownThreshhold)
        { // but not much stronger...
            if (strongerCharacter)
            {
                EnterChamberState(strongerCharacter);
                strongerCharacter->Logger.Log(this, "Chamber: chamber anim!");
            }
        } // else continue with attack
        else
            enemyStronger ? HitCharacter->Logger.Log(HitCharacter->CombatComponent, "Chamber: continue with attack!") : Owner->Logger.Log(this, "Chamber: continue with attack!");
        // define chamber state for this character
        if (strongerCharacter == Owner || staggeredCharacter == Owner)
            state = EChamberRes::ChamberState;
        else
            state = EChamberRes::ContinueAttack;
    }
    else
    { // both in chamber
        if (characterIsInterruptable)
        {
            EnterChamberState(Owner);
            state = EChamberRes::ChamberState;
        }
        else
            state = EChamberRes::ContinueAttack;
        if (enemyIsInterruptable)
            EnterChamberState(HitCharacter);
        Owner->Logger.Log(this, "Chamber: should be chamber for both!");
    }
    return state;
}

void UCombatComponent::EnterChamberState(ACharacterCreature* ChamberedCharacter)
{
    ChamberedCharacter->CombatComponent->GetCombatFSM().state(ECombatState::Attack)->Info.ResetAnimInfo();
    ChamberedCharacter->CombatComponent->GetCombatFSM().state(ECombatState::Attack)->Info.ChangeAnimInfo(ECombatStateAnimInfo::CheckedForChamber, true);
    ChamberedCharacter->CombatComponent->GetCombatFSM().state(ECombatState::Attack)->Info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
    ChamberedCharacter->InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::Chamber);
}

void UCombatComponent::OnEquipmentUpdate(UEquipmentComponent* Component, EItemSlot Slot)
{ // TODO fix crash when unequipping main hand while has offhand
    auto item = Component->GetEquipmentFromSlot(Slot);
    if ((Slot == EItemSlot::MainWeaponSlot) || (Slot == EItemSlot::OffMeaponSlot))
    {
        auto result = ResolveNewStance(nullptr, true);
        if (result.NoValidStanceFound)
        {
            // check if has valid combat stance for equipped weapon, if not cancel
            if (item.Item)
            {
                auto unfitItem = Owner->EquipmentComponent->Unequip(Slot);
                Owner->InventoryComponent->AddToInventory(unfitItem, 1, true); // TODO until we think of something better
            }
        }
        else if (!result.NewStanceWasSet)
        { // if stance has not changed but weapon did -> setup new weapon
            CurrentCombatStance->SetWeaponVars(Owner);
            CurrentCombatStance->OnStanceActivated();
            CurrentCombatStance->ArmsSetup(IsWeaponDrawn);
        }
    }
}

void UCombatComponent::OnEquipmentSwap(UEquipmentComponent* Component, EItemSlot SlotL, EItemSlot SlotR)
{
    auto itemL = Component->GetEquipmentFromSlot(SlotL);
    auto itemR = Component->GetEquipmentFromSlot(SlotR);
    if (!itemL.Item || !itemR.Item)
        return;
    // TODO check if weapon slots and reinitialize current stance to swap weapons or resolve new stance
    // call CurrentCombatStance->ArmsSetup(IsWeaponDrawn)
}

bool UCombatComponent::IsWeaponSwapAllowed(EItemSlot lhs, EItemSlot rhs) const
{ // TODO
    return false;
}

void UCombatComponent::Update()
{
    // TODO what?
    UnSetUpdate();
}

void UCombatComponent::OnAttack(bool Started)
{
    Owner->Logger.Log(this, *FString::Printf(TEXT("OnAttack %s"),
            Started ? TEXT("pressed") : TEXT("released")));
    AttackPressed = Started;
    if (!HasCombatStance())
        return;
    if (CurrentCombatStance->IsMelee())
    {
        if (GetCombatFSM().isInState(ECombatState::Attack))
        {
            auto& info = GetCombatFSM().state(ECombatState::Attack)->Info;
            const FAttackInfo* currentAttack = info.AttackInfo;
            if (Owner->CCStateManager.IsInCCMode(ECCState::LossOfBalance) && Owner->CCStateManager.IsInCCSubMode(Chamber))
            {
                if (!info.CheckAnimInfo(CanCounterChamber)) // check if not ready to strike while in chamber
                    return;
                if (AttackPressed)
                {
                    info.ChangeAnimInfo(ECombatStateAnimInfo::ComboSaved, true);
                    StartNextComboAttack(false);
                    return;
                }
            }
            if (currentAttack->Special == ESpecialType::None)// can check combo if not special move
            {
                if (AttackPressed)
                {
                    if (info.CheckAnimInfo(ECombatStateAnimInfo::ComboCanContinue))// start pause combo
                        StartNextComboAttack(true);
                    else
                        info.ChangeAnimInfo(ECombatStateAnimInfo::ComboSaved, true);
                }
                else
                {
                    if (info.ChargeStage != -1 && info.CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed))
                    {
                        auto meleeStance = Cast<UMeleeCombatStance>(CurrentCombatStance);
                        bool hasStamina = Owner->AttributeComponent->Stamina->CurrentValue > 0;
                        auto airAttack = meleeStance->GetAttackInfoForSpecial(ESpecialType::Air);
                        if (Owner->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Falling && hasStamina && airAttack != nullptr)
                        { // uncharging in air
                            info.AttackInfo = meleeStance->GetAttackInfoForSpecial(ESpecialType::Air);
                            info.ResetAnimInfo();
                            info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                            Owner->AttributeComponent->Stamina->Consume(info.AttackInfo->StaminaCost);
                            FOnMontageBlendingOutStarted callBack;
                            callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
                            Owner->StartAnimation(info.AttackInfo->AttackMontage, callBack, AttackSpeed);
                            return;
                        }
                        else if (hasStamina && currentAttack->AttackMontage && info.ChargeTime >=
                            meleeStance->ComboAttacks[meleeStance->GetComboCount()].ChargeAttackStages[GetCombatFSM().currentState()->Info.ChargeStage].ChargeTimeNeeded)
                        { // uncharging on ground
                            Owner->AttributeComponent->Stamina->Consume(currentAttack->StaminaCost);
                            info.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, false);
                            info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                            FOnMontageBlendingOutStarted callBack;
                            callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
                            Owner->StartAnimation(currentAttack->AttackMontage, callBack, AttackSpeed, FAnimSectionNames::End);
                            return;
                        }
                        // charging cancelled
                        info.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, false);
                        info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                        FOnMontageBlendingOutStarted callBack;
                        callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
                        Owner->StartAnimation(currentAttack->AttackMontage, callBack, AttackSpeed, FAnimSectionNames::Cancel);
                    }
                }
            }
        }
        else if (AttackPressed)// starting first attack
            UpdateCombatFSM();
    }
}

void UCombatComponent::OnBlock(bool Started) 
{
    Owner->Logger.Log(this, *FString::Printf(TEXT("OnBlock %s"),
            Started ? TEXT("pressed") : TEXT("released")));
    BlockPressed = Started;
    if (Owner->CCStateManager.IsInCCMode(ECCState::LossOfBalance) && 
        Owner->CCStateManager.IsInCCSubMode(BlockedAttack))
        return;
    if (!BlockPressed && GetCombatFSM().isInState(ECombatState::Block)) // end block
    {
        GetCombatFSM().currentState()->Info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(this, &UCombatComponent::OnBlockEnded);
        Owner->StartAnimation(Owner->AnimInstance->CurrentBlockAnim, callBack, AttackSpeed, FAnimSectionNames::End);
    }
    else
        UpdateCombatFSM();
}

void UCombatComponent::OnDrawWeapon()
{
    if (CurrentCombatStance)
    {
        WeaponSwitchPressed = true;
        UpdateCombatFSM();
        WeaponSwitchPressed = false;
    }
    else
        Owner->Logger.Log(this, "Can't draw weapon: Invalid combat stance");
}

void UCombatComponent::OnUseAbility(bool Started)
{
    AbilityPressed = Started;
    Owner->Logger.Log(this,
                      *FString::Printf(TEXT("OnAbility %s"),
                                       Started ? TEXT("pressed") : TEXT("released")));
    if (Owner->CCStateManager.HasCC())
        return;

    if (CurrentAbility != nullptr)
    {
        if (AbilityPressed)
        {
            if (!CurrentAbility->bIsActive)
                UpdateCombatFSM();
        }
        else
            ReleaseAbility();
    }
}

void UCombatComponent::StartNextComboAttack(bool IsPauseAttack)
{
    check(GetCombatFSM().isInState(ECombatState::Attack) && "StartNextComboAttack: not in attack state");
    check(CurrentCombatStance && "StartNextComboAttack: doesn't have stance");
    auto meleeStance = Cast<UMeleeCombatStance>(CurrentCombatStance);
    int CurrentCombo = meleeStance->GetComboCount();
    if (CurrentCombo >= meleeStance->ComboAttacks.Num() - 1)
        return;
    if (Owner->AttributeComponent->Stamina->CurrentValue <= 0)
        return;

    auto& stateInfo = GetCombatFSM().currentState()->Info;
    FAttackInfo* nextAttackInfo = nullptr;
    if (IsPauseAttack)
    {
        if (meleeStance->ComboAttacks[CurrentCombo + 1].PauseAttack.AttackMontage != nullptr)
        {
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::PauseComboSaved, true);
            nextAttackInfo = &meleeStance->ComboAttacks[CurrentCombo + 1].PauseAttack;
        }
        else
            return;
    }
    else
    {
        if (stateInfo.CheckAnimInfo(ECombatStateAnimInfo::ComboSaved))
        {
            if (meleeStance->ComboAttacks[CurrentCombo + 1].NormalAttack.AttackMontage != nullptr)
                nextAttackInfo = &meleeStance->ComboAttacks[CurrentCombo + 1].NormalAttack;
        }
        else
            return;
    }
    if (nextAttackInfo != nullptr)
    {
        bool isAttackFromChamber = stateInfo.CheckAnimInfo(ECombatStateAnimInfo::CanCounterChamber);
        meleeStance->IncreaseComboCount();
        Owner->AttributeComponent->Stamina->Consume(nextAttackInfo->StaminaCost);
        stateInfo.ResetAnimInfo();
        CharactersHit.Reset();
        auto activeWeapons = FindActiveMeleeWeapons();
        for (auto& weaponActor : activeWeapons)
            SetWeaponCollision(weaponActor, false);
        Owner->AnimInstance->OnMovementAllowed(false);
        stateInfo.AttackInfo = nextAttackInfo;
        if (isAttackFromChamber)
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::CanCounterChamber, true);
        else
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, true);
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
        Owner->StartAnimation(stateInfo.AttackInfo->AttackMontage, callBack, AttackSpeed, stateInfo.AttackInfo->AnimSectionName);
    }
}

void UCombatComponent::ToggleHandMode()
{
    if (IsBusy())
        return;

    TwoHandMode = !TwoHandMode;
    UCombatStance* newStance = CheckStances(StanceStack, true);
    UCombatStance* defaultStance = CheckStances(Owner->AnimInstance->DefaultStances, true);
    if (newStance == nullptr)
    {
        if (defaultStance == nullptr)
            Owner->Logger.Log(this, "ToggleHandMode: no valid stance to switch to, retaining the current one");
        else
            newStance = defaultStance; // apply default stance
    }
    else
        StanceStack.Insert(newStance, 0);
    if ((newStance != nullptr) && (newStance != CurrentCombatStance))
        StanceChanged(CurrentCombatStance, newStance, defaultStance, false);
    // TODO finish stack logic
}

bool UCombatComponent::HasAlreadyHitTarget(AActor *Target, UItemArms* Weapon) const
{
    if (Target)
        return CharactersHit.Contains(TPair<AActor*, UItemArms*>(Target, Weapon));
    return false;
}

float UCombatComponent::GetAngleBetweenAttackerAndDefender(const AActor *Attacker, const AActor *Defender)
{
    check(Attacker != nullptr && Defender != nullptr && "GetAngleBetweenAttackerAndDefender: invalid arg");
    const FVector defenderV = FRotator(0, Defender->GetActorRotation().Yaw, 0).Vector();
    const FVector attackerV = UKismetMathLibrary::GetDirectionUnitVector(Attacker->GetActorLocation(), Defender->GetActorLocation());
    float direction = FVector::DotProduct(attackerV, defenderV);
    return FMath::Abs(acos(direction) * 180.f / PI - 180.f);
}

float UCombatComponent::GetAngleBetweenActors(const AActor *First, const AActor *Second)
{
    check(First != nullptr && Second != nullptr && "GetAngleBetweenActors: invalid arg");
    const FVector firstV = FRotator(0, First->GetActorRotation().Yaw, 0).Vector();
    const FVector secondV = FRotator(0, Second->GetActorRotation().Yaw, 0).Vector();
    float direction = FVector::DotProduct(firstV, secondV);
    return FMath::Abs(acos(direction) * 180.f / PI - 180.f);
}

void UCombatComponent::OnWeaponSwitchEnded(UAnimMontage* animMontage, bool bInterrupted)
{
    auto& stateInfo = GetCombatFSM().state(ECombatState::WeaponSwitch)->Info;
    IsWeaponDrawn = !IsWeaponDrawn;
    if (!stateInfo.CheckAnimInfo(ECombatStateAnimInfo::WeaponGrabbed))
        CurrentCombatStance->ArmsSetup(IsWeaponDrawn);
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
    UpdateCombatFSM();
}

void UCombatComponent::OnBlockEnded(UAnimMontage* animMontage, bool bInterrupted)
{
    if (bInterrupted && GetCombatFSM().isInState(ECombatState::Parry))// interrupted by Parry
        return;
    auto& stateInfo = GetCombatFSM().state(ECombatState::Block)->Info;
    if (stateInfo.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt))// to exit loop section
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, false);
    else
    {
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
        UpdateCombatFSM();
    }
}

void UCombatComponent::OnParryEnded(UAnimMontage* animMontage, bool bInterrupted)
{
    auto& stateInfo = GetCombatFSM().state(ECombatState::Parry)->Info;
    if (GetCombatFSM().isInState(ECombatState::Parry))
    {
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
        UpdateCombatFSM();
    }
}

void UCombatComponent::OnAttackEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    if (GetCombatFSM().currentState()->Id != ECombatState::Attack)// interrupted
        return;
    auto& stateInfo = GetCombatFSM().state(ECombatState::Attack)->Info;
    if (stateInfo.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt))
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, false);// continued combo
    else
    {
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
        UpdateCombatFSM();
    }
}

UCombatStance* UCombatComponent::CheckStances(const TArray<UCombatStance*>& Stances, bool CheckHandMode)
{
    for (auto stance : Stances)
    {
        if (DoesStanceFit(stance, CheckHandMode))
            return stance;
    }
    return nullptr;
}

bool CheckHitActorCollision(const AItemActor* WeaponActor, const UPrimitiveComponent* HitComp)
{
    return (WeaponActor != nullptr) && (WeaponActor->ItemCollision == HitComp);
}

UItemArms* UCombatComponent::FindHitMeleeWeapon(const UPrimitiveComponent* HitComp) const
{
    AItemActor* weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::MainHand);
    if (CheckHitActorCollision(weaponActor, HitComp))
        return Cast<UItemArms>(weaponActor->ItemBase);
    weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::OffHand);
    if (CheckHitActorCollision(weaponActor, HitComp))
        return Cast<UItemArms>(weaponActor->ItemBase);
    weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::RightFoot);
    if (CheckHitActorCollision(weaponActor, HitComp))
        return Cast<UItemArms>(weaponActor->ItemBase);
    weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::LeftFoot);
    if (CheckHitActorCollision(weaponActor, HitComp))
        return Cast<UItemArms>(weaponActor->ItemBase);

    return nullptr;
}

bool UCombatComponent::HitOwnMeleeWeapons(const UPrimitiveComponent* HitComp, const AActor* HitActor) const
{
    auto weaponActor = Cast<AItemActor>(HitActor);
    if (weaponActor)
        return weaponActor->ItemCollision == HitComp;

    return false;
}

bool CheckActorCollisionActive(const AItemActor* WeaponActor)
{
    return (WeaponActor != nullptr) && (WeaponActor->ItemCollision->GetGenerateOverlapEvents());
}

UItemArms* UCombatComponent::FindCharacterActiveWeapon(const ACharacterCreature* SomeCharacter) const
{
    auto stance = SomeCharacter->CombatComponent->CurrentCombatStance;
    if (stance)
    {
        AItemActor* weaponActor = stance->GetArmsActorFromSlot(EArmsSlot::MainHand);
        if (CheckActorCollisionActive(weaponActor))
            return Cast<UItemArms>(weaponActor->ItemBase);
        weaponActor = stance->GetArmsActorFromSlot(EArmsSlot::OffHand);
        if (CheckActorCollisionActive(weaponActor))
            return Cast<UItemArms>(weaponActor->ItemBase);
        weaponActor = stance->GetArmsActorFromSlot(EArmsSlot::RightFoot);
        if (CheckActorCollisionActive(weaponActor))
            return Cast<UItemArms>(weaponActor->ItemBase);
        weaponActor = stance->GetArmsActorFromSlot(EArmsSlot::LeftFoot);
        if (CheckActorCollisionActive(weaponActor))
            return Cast<UItemArms>(weaponActor->ItemBase);
    }
    
    return nullptr;
}

TArray<AItemActor*> UCombatComponent::FindActiveMeleeWeapons() const
{
    TArray<AItemActor*> result;
    AItemActor* weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::MainHand);
    if (CheckActorCollisionActive(weaponActor))
        result.Push(weaponActor);
    weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::OffHand);
    if (CheckActorCollisionActive(weaponActor))
        result.Push(weaponActor);
    weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::RightFoot);
    if (CheckActorCollisionActive(weaponActor))
        result.Push(weaponActor);
    weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::LeftFoot);
    if (CheckActorCollisionActive(weaponActor))
        result.Push(weaponActor);

    return result;
}

FStanceResolveResult UCombatComponent::ResolveNewStance(UCombatStance* NewStance, bool WeaponChanged)
{
    UCombatStance* newStance;
    if (NewStance)
    {
        if (DoesStanceFit(NewStance))
            newStance = NewStance;
        else
            return FStanceResolveResult{false, false}; // if can't set specified stance just return negative result;
    }
    else
        newStance = CheckStances(StanceStack);
    FStanceResolveResult result;
    UCombatStance* defaultStance = CheckStances(Owner->AnimInstance->DefaultStances);
    if (newStance == nullptr)
    {
        if (defaultStance == nullptr)
        {
            if (!WeaponChanged)
            { // if during equipment update no new stance was resolved then reinit the current one
                Owner->Logger.Log(this, "no valid default stance on character's anim instance");
                result.NoValidStanceFound = true;
            }
            return result;
        }
        else
            newStance = defaultStance; // apply default stance
    }
    //else
    //    StanceStack.Insert(newStance, 0); // TODO finish stack logic
    if (newStance != CurrentCombatStance)
    {
        if (NewStance && CurrentCombatStance)
        {
            if (newStance->GetClass() == CurrentCombatStance->GetClass())
                return result;
        }
        StanceChanged(CurrentCombatStance, newStance, defaultStance, WeaponChanged);
        result.NewStanceWasSet = true;
    }
    return result;
}

bool UCombatComponent::DoesStanceFit(UCombatStance* Stance, bool CheckHandMode) const
{
    if (CurrentCombatStance == Stance)
        return false;
    Stance->SetWeaponVars(Owner);
    bool correctHandMode = true;
    if (CheckHandMode)
    {
        correctHandMode =  TwoHandMode ? (Stance->StanceType == EWeaponCombination::TwoHand) : 
            (Stance->StanceType != EWeaponCombination::TwoHand);
    }
    bool result = Stance->IsStanceValid(Owner) && Stance->CheckRequirement(Owner) && correctHandMode;
    if (!result)
        Stance->ResetWeaponVars();
    return result;
}

void UCombatComponent::ResetRotationRate()
{
    if (IsWeaponDrawn)
    {
        if (IsInActionState())
            Owner->GetMovementComponent()->RotationRate.Yaw = ActionRotationRate;
        else
            Owner->GetMovementComponent()->RotationRate.Yaw = CombatModeRotationRate;
    }
    else
        Owner->GetMovementComponent()->RotationRate.Yaw = Owner->GetMovementComponent()->DefaultRotationRate;
}

void UCombatComponent::SetNewStance(UCombatStance* OldStance, UCombatStance* NewStance, const UCombatStance* DefaultStance)
{
    bool isSwitchPending = PendingStance.IsSet();
    if (OldStance)
    {
        OldStance->ResetStance();
    }
    CurrentCombatStance = NewStance;
    if (NewStance)
    {
        TwoHandMode = NewStance->StanceType == EWeaponCombination::TwoHand ? true : false;
        NewStance->StanceSwitched(Owner, DefaultStance);
        if (isSwitchPending)
            UpdateCombatFSM();
    }
    else if (isSwitchPending)
        PendingStance.Reset();
}

void UCombatComponent::StanceChanged(UCombatStance* OldStance, UCombatStance* NewStance, const UCombatStance* DefaultStance, bool WeaponChanged)
{ // if has weapons drawn and need to switch weapons, then first queue weapon sheath and then init new stance
    if (IsWeaponDrawn && WeaponChanged)
    {
        PendingStance.NewStance = NewStance;
        PendingStance.DefaultStance = DefaultStance;
        UpdateCombatFSM();
    }
    else
    {
        SetNewStance(OldStance, NewStance, DefaultStance);
    }
}

void UCombatComponent::CalculateMaxCombatSpeeds()
{
    float speedMod = Owner->GetMovementComponent()->GetSpeedModifier();
    Owner->GetMovementComponent()->MaxRunSpeed =
        CurrentCombatStance->MaxCombatRunSpeed * speedMod;
    Owner->GetMovementComponent()->MaxSprintSpeed =
        CurrentCombatStance->MaxCombatSprintSpeed * speedMod;
}

void UCombatComponent::CalculateMaxBlockingSpeed(float SpeedModifier)
{
    Owner->GetMovementComponent()->MaxRunSpeed = CurrentCombatStance->MaxBlockOrActionSpeed * SpeedModifier;
}

bool UCombatComponent::CanBeInterruptedWithBlock(int CurrentState) const
{
    if (CurrentCombatStance->IsMelee())
    {
        const auto state = GetCombatFSM().state(CurrentState);
        return !state->Info.CheckAnimInfo(ECombatStateAnimInfo::CollisionOn) && BlockPressed;
    }
    else
        return true;
}

void UCombatComponent::WeaponSetup(UItemArms* Weapon, AItemActor* Actor, bool Draw, FName Slot, bool MainHand)
{
    if (Slot.IsNone())
        Slot = GetSlotFromWeaponType(Weapon->WeaponType, Draw, MainHand);
    if (Draw)
        Actor->ItemCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &UCombatComponent::OnWeaponOverlap);
    else
        Actor->ItemCollision->OnComponentBeginOverlap.RemoveDynamic(this, &UCombatComponent::OnWeaponOverlap);
    SetWeaponCollision(Actor, false); // disable equipped weapon collision
    Actor->AttachToComponent(Owner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Slot);
}

void UCombatComponent::SetWeaponCollision(AItemActor* WeaponActor, bool Enable)
{
    WeaponActor->ItemCollision->SetGenerateOverlapEvents(Enable);
}

void UCombatComponent::ActivateAbility()
{
    if(CurrentAbility->IsValidLowLevelFast())
    {
        CurrentAbility->ActivateAbility();
        Owner->SetAbilityCameraMode(CurrentAbility->AbilityCameraMode);
    }
}

void UCombatComponent::ReleaseAbility()
{
    if(CurrentAbility->IsValidLowLevelFast())
        CurrentAbility->ReleaseAbility();
}

void UCombatComponent::AddAbility(UAbstractAbility *Ability)
{
    AbilityList.Emplace(Ability);
}

void UCombatComponent::RemoveAbility(UAbstractAbility *Ability)
{
    AbilityList.Remove(Ability);
}

void UCombatComponent::SetCurrentAbility(UAbstractAbility *Ability)
{
    if(CurrentAbility)
    {
        CurrentAbility->CancelAbility();
    }

    CurrentAbility = Ability;
    if(CurrentAbility)
        CurrentAbility->AbilitySetup(this);
    else
    {
        Owner->Logger.Log(this, *FString::Printf(TEXT("AbilitySystem Component: Ability Is Null")));
    }
}

void UCombatComponent::CalculateMaxCastingSpeed(float SpeedModifier)
{
    Owner->GetMovementComponent()->MaxRunSpeed = MaxSpeedWhileCasting * SpeedModifier;
}

void UCombatComponent::UseAimComponent(class UAbilityAimComponent *Component)
{
    Owner->UseAimComponent(Component);
}

bool UCombatComponent::IsUsingAbility() const
{
    return HasCombatStance() && GetCombatFSM().isInState(ECombatState::Ability);
}
