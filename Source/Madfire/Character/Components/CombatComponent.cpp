#include "CombatComponent.h"

#include "../CharacterCreature.h"
#include "Kismet/KismetMathLibrary.h"
#include "Items/RangedWeaponAnimInstance.h"

UCombatComponent::UCombatComponent(const class FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    InitFSM();
    // TODO temp test
    AttackSpeed = 1.f;// TODO set in update from dex mod?
    UnarmedBaseDamage = 1.f;
    BlockStaminaRegenRateMod = 0.5f;
    CombatModeRotationRate = 500.f;
    ActionRotationRate = 100.f;
    ChargeStartTime = 0.3f;
    BlockStaminaCost = 10.f;
    KnockdownThreshhold = 1.5f;
    ChamberThreshhold = 1.3f;
    MaxFacingAngle = 45.f;
    ShieldWeightDamageMod = 0.5f;
    DrawTimeModFromTension = 1500.f;
    CameraMoveSpreadAddTimeOut = 1.0;
    LastChargeTimeStaminaConsume = 0.f;
    // end temp
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
        auto gloves = Owner->GetEquipmentComponent()->GetEquipmentFromSlot(EItemSlot::HandsSlot).Item;
        if (gloves)
            return FMath::Max((UnarmedBaseDamage + gloves->ItemBase->Weight) + strMod, UnarmedBaseDamage);
        else
            return FMath::Max(UnarmedBaseDamage + strMod, UnarmedBaseDamage);
    }
    case EWeaponType::Shield:
        return FMath::Max(UnarmedBaseDamage, UnarmedBaseDamage + strMod + (weapon->Weight * ShieldWeightDamageMod));
    default:  // includes ranged weapon melee damage
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
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // update spread if has ragned weapon equipped
    if (auto rangedWeapon = GetEquippedRangedWeapon())
        rangedWeapon->Tick(DeltaTime, Owner);
    auto currentState = CombatFSM.currentState();
    if (CurrentCombatStance && CombatFSM && currentState->Id == ECombatState::Attack && AttackPressed)  // check if charging
    {
        if (currentState->CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed))
        {
            currentState->ChargeTime += DeltaTime;
            if (CurrentCombatStance->IsMelee())
            {
                // check if held attack for at least some time to start charging and has charge stage
                if (currentState->ChargeStage == -1 && currentState->ChargeTime < ChargeStartTime)
                    return;

                auto meleeStance = Cast<UMeleeCombatStance>(CurrentCombatStance);
                auto& currentCombo = meleeStance->ComboAttacks[GetComboCount()];

                if (currentCombo.ChargeAttackStages.Num() == 0)
                    return;

                // charged enough to go to next stage if has any
                if (currentState->ChargeStage == -1 || ((currentState->ChargeStage + 1 < currentCombo.ChargeAttackStages.Num()) && currentState->ChargeTime >=
                    currentCombo.ChargeAttackStages[currentState->ChargeStage + 1].ChargeTimeNeeded))
                {
                    if (currentCombo.ChargeAttackStages[currentState->ChargeStage + 1].Info.AttackMontage != nullptr)
                    {
                        ++currentState->ChargeStage;
                        currentState->ResetAnimInfo();
                        currentState->AttackInfo = &currentCombo.ChargeAttackStages[currentState->ChargeStage].Info;
                        currentState->ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                        currentState->ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, true);
                        auto callBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnAttackEnded);
                        Owner->StartAnimation(currentState->AttackInfo->AttackMontage, callBack, AttackSpeed, currentState->AttackInfo->AnimSectionName);
                    }
                }
            }
            else
            {
                auto stance = Cast<UBowCombatStance>(CurrentCombatStance);

                if (stance && (currentState->ChargeTime - LastChargeTimeStaminaConsume >= stance->ChargeStaminaConsumeDelta))
                {
                    LastChargeTimeStaminaConsume = currentState->ChargeTime;
                    if (!Owner->GetAttributeComponent()->Stamina->Consume(stance->AttackInfo.StaminaCost))
                    {
                        // draw cancelled
                        currentState->ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, false);
                        currentState->ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                        auto callBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnAttackEnded);
                        Owner->StartAnimation(stance->AttackInfo.AttackMontage, callBack, 1.f, FAnimSectionNames::Cancel);
                    }
                }
            }
        }
    }
}

bool UCombatComponent::UpdateCombatFSM()
{    
    if(!CombatFSM)
        return false;

    int oldState = CombatFSM.currentState()->Id;
    auto newState = CombatFSM.update();
    if(newState)
    {
        CombatFSM.setCurrentState(newState->Id);
        OnCombatStateChanged.Broadcast(oldState, newState->Id);
    }

    return newState != nullptr;
}

void UCombatComponent::ResetActionState(int StateId)
{
    // reset root motion ignore
    Owner->AnimInstance->OnMovementAllowed(true);
    Owner->AnimInstance->OnRotationAllowed(false);
    Owner->GetAttributeComponent()->Stamina->UnblockRegen();
    CombatFSM[StateId].ResetAnimInfo();
    Owner->GetMovementComponent()->RotationRate.Yaw = CombatModeRotationRate;
}

void UCombatComponent::OnMovementModeChanged(int oldMode, int newMode)
{ 
    if (CurrentCombatStance && CombatFSM.currentState() && CombatFSM.isInState(ECombatState::Attack))
    {
        // check if started falling during regular attack
        if (newMode == EMovementMode::MOVE_Falling && !CombatFSM.currentState()->CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed))
            Owner->InterruptAnimation(ECCState::NoCC);

        // check if landed during air attack to finish anim
        if (newMode == EMovementMode::MOVE_Walking && CombatFSM.currentState()->AttackInfo->Special == ESpecialType::Air)
        {
            CombatFSM.currentState()->ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
            auto callBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnAttackEnded);
            Owner->StartAnimation(CombatFSM.currentState()->AttackInfo->AttackMontage, callBack, AttackSpeed, FAnimSectionNames::End);	
        }
    }
}

void UCombatComponent::OnWeaponOverlap(UPrimitiveComponent* comp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    auto hitActor = SweepResult.GetActor();

    if (hitActor == nullptr || OtherActor == Owner || HitOwnMeleeWeapons(comp, OtherActor))// Check self and weapon
        return;

    if (CombatFSM.is(ECombatState::Attack))
    {
        const auto &info = CombatFSM[ECombatState::Attack];
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
                    //Owner->Logger.Log(this, *FString::Printf(TEXT("OnBeginOverlap: invul character!")));
                    return;
                }

                // check for chamber scenario
                auto hitCombatCharacter = Cast<ACharacterCreature>(hitActor);
                auto enemyActiveWeapons = hitCombatCharacter->GetCombatComponent()->FindActiveMeleeWeapons();

                for(auto &weapon : enemyActiveWeapons)
                    if (hitCombatCharacter && IsChamberValid(hitCombatCharacter, Cast<UItemArms>(weapon->ItemBase), activeWeapon))
                    {
                        if (ResolveChamber(hitCombatCharacter, Cast<UItemArms>(weapon), activeWeapon))
                            return;
                    }

                int32 strMod = UAttributeSetHelper::GetModifier(hitCharacter->GetAttributeComponent()->Attributes, EAttributes::Str);
                const float damage = CalculateWeaponDamage(activeWeapon, CombatFSM[ECombatState::Attack].AttackInfo->DamageMod, strMod);
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

                    }
                }

                hitCharacter->TakeDamage(damage, FForceDamageEvent(activeWeapon->DamageType, force), Owner->GetController(), Owner);
            }
            else // hit an obsticle
            {
                if (SweepResult.GetComponent()->GetCollisionObjectType() == ECollisionChannel::ECC_WorldStatic) // check if hit a wall and interrupt with loss of balance
                {
                    Owner->InterruptAnimation(ECCState::LossOfBalance, static_cast<int>(ELossOfBalanceCCSubState::HitWall));
                }
                else // try to damage the actor
                {
                    float damage = CalculateWeaponDamage(activeWeapon, CombatFSM[ECombatState::Attack].AttackInfo->DamageMod, 0);
                    hitActor->TakeDamage(damage, FDamageEvent(activeWeapon->DamageType), Owner->GetController(), Owner);
                }
            }
            if (info.AttackInfo && info.AttackInfo->CustomOnHitEvent.IsBound())
                info.AttackInfo->CustomOnHitEvent.Broadcast(Owner, hitActor);
        }
    }
}

void UCombatComponent::OnRangedWeaponOverlap(UPrimitiveComponent* comp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{  // TODO refactor
    auto hitActor = SweepResult.GetActor();

    if (hitActor == nullptr || OtherActor == Owner || HitOwnMeleeWeapons(comp, OtherActor))// Check self and weapon
        return;

    if (CombatFSM.is(ECombatState::Parry))
    {
        const auto& info = CombatFSM[ECombatState::Parry];
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
                    return;
                }

                int32 strMod = UAttributeSetHelper::GetModifier(hitCharacter->GetAttributeComponent()->Attributes, EAttributes::Str);
                const float damage = CalculateWeaponDamage(activeWeapon, 1.f, strMod);
                float force = CalculateHitForce(activeWeapon->Weight, strMod, 1.f); // Use Force, Luke, again
                hitCharacter->TakeDamage(damage, FForceDamageEvent(activeWeapon->DamageType, force), Owner->GetController(), Owner);
            }
            else // hit an obsticle
            {
                if (SweepResult.GetComponent()->GetCollisionObjectType() == ECollisionChannel::ECC_WorldStatic) // check if hit a wall and interrupt with loss of balance
                {
                    Owner->InterruptAnimation(ECCState::LossOfBalance, static_cast<int>(ELossOfBalanceCCSubState::HitWall));
                }
                else // try to damage the actor
                {
                    float damage = CalculateWeaponDamage(activeWeapon, 1.f, 0);
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
    return CombatFSM.is(ECombatState::Attack) || CombatFSM.currentState()->Id == ECombatState::Parry;
}

bool UCombatComponent::IsChamberValid(const ACharacterCreature* HitCharacter, const UItemArms* HitActorActiveWeapon, const UItemArms* OwnerActiveWeapon) const
{
    if (HitCharacter->GetCombatComponent()->CombatFSM.is(ECombatState::Attack))
    {
        auto& enemyAttackInfo = HitCharacter->GetCombatComponent()->CombatFSM[ECombatState::Attack];
        auto& attackInfo = CombatFSM[ECombatState::Attack];
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

bool UCombatComponent::ResolveChamber(ACharacterCreature* HitCharacter, const UItemArms* HitActorActiveWeapon, const UItemArms* OwnerActiveWeapon)
{
    // calculate characters attack powers
    int32 strMod = UAttributeSetHelper::GetModifier(Owner->GetAttributeComponent()->Attributes, EAttributes::Str);
    auto& characterAttackInfo = CombatFSM[ECombatState::Attack];
    float force = CalculateHitForce(OwnerActiveWeapon->Weight, strMod, characterAttackInfo.AttackInfo->PowerMod);

    int32 enemyStrMod = UAttributeSetHelper::GetModifier(HitCharacter->GetAttributeComponent()->Attributes, EAttributes::Str);
    auto& enemyAttackInfo = HitCharacter->GetCombatComponent()->CombatFSM[ECombatState::Attack];
    float enemyForce = CalculateHitForce(HitActorActiveWeapon->Weight, enemyStrMod, enemyAttackInfo.AttackInfo->PowerMod);

    bool enemyStronger = enemyForce > force;
    float forceRatio = enemyStronger ? enemyForce/force : force/enemyForce;

    characterAttackInfo.ChangeAnimInfo(ECombatStateAnimInfo::CheckedForChamber, true);
    enemyAttackInfo.ChangeAnimInfo(ECombatStateAnimInfo::CheckedForChamber, true);
    bool characterIsInterruptable = !characterAttackInfo.AttackInfo->IsNotInterruptable;
    bool enemyIsInterruptable = !enemyAttackInfo.AttackInfo->IsNotInterruptable;
    //EChamberRes state;
    bool state;

    if (forceRatio > ChamberThreshhold)
    { // one is stronger than other
        // apply reaction to weaker one
        auto staggeredCharacter = enemyStronger ? (characterIsInterruptable ? Owner : nullptr) : (enemyIsInterruptable ? HitCharacter : nullptr);
        if (staggeredCharacter)
        {
            staggeredCharacter->InterruptAnimation(ECCState::LossOfBalance, static_cast<int>(ELossOfBalanceCCSubState::None));
        }
        // apply reaction to stronger one
        auto strongerCharacter = enemyStronger ? (enemyIsInterruptable ? HitCharacter : nullptr) : (characterIsInterruptable ? Owner : nullptr);
        if (forceRatio <= KnockdownThreshhold)
        { // but not much stronger...
            if (strongerCharacter)
            {
                EnterChamberState(strongerCharacter);
                //strongerCharacter->Logger.Log(this, "Chamber: chamber anim!");
            }
        } // else continue with attack

        // define chamber state for this character
        if (strongerCharacter == Owner || staggeredCharacter == Owner)
            return true;
        else
            return false;
    }
    else
    { // both in chamber
        if (characterIsInterruptable)
        {
            EnterChamberState(Owner);
            state = true;
        }
        else
            state = false;
        if (enemyIsInterruptable)
            EnterChamberState(HitCharacter);
    }
    return state;
}

void UCombatComponent::EnterChamberState(ACharacterCreature* ChamberedCharacter)
{
    CombatFSM[ECombatState::Attack].ResetAnimInfo();
    CombatFSM[ECombatState::Attack].ChangeAnimInfo(ECombatStateAnimInfo::CheckedForChamber, true);
    CombatFSM[ECombatState::Attack].ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
    ChamberedCharacter->InterruptAnimation(ECCState::LossOfBalance, static_cast<int>(ELossOfBalanceCCSubState::Chamber));
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
                auto unfitItem = Owner->GetEquipmentComponent()->Unequip(Slot);
                Owner->GetInventoryComponent()->AddToInventory(unfitItem, 1, true); // TODO until we think of something better
            }
        }
        else if (!result.NewStanceWasSet && CurrentCombatStance)
        { // if stance has not changed but weapon did -> setup new weapon
            //CurrentCombatStance->SetWeaponVars(Owner);            
            CurrentCombatStance->OnStanceActivated();
            //CurrentCombatStance->ArmsSetup(IsWeaponDrawn);
            ArmsSetup(IsWeaponDrawn);
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
    SetUpdate(false);
}

void UCombatComponent::OnAttack(bool Started)
{
    AttackPressed = Started;
    if (!HasCombatStance())
        return;

    if (CurrentCombatStance->IsMelee())
    {
        if (CombatFSM.is(ECombatState::Attack))
        {
            auto& info = CombatFSM[ECombatState::Attack];            
            if (Owner->GetССComponent()->CheckCCMode(ECCState::LossOfBalance, static_cast<int>(ELossOfBalanceCCSubState::Chamber)))
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

            const FAttackInfo* currentAttack = info.AttackInfo;

            if (currentAttack->Special == ESpecialType::None) // can check combo if not special move
            {
                if (AttackPressed)
                {
                    if (info.CheckAnimInfo(ECombatStateAnimInfo::ComboCanContinue)) // start pause combo
                        StartNextComboAttack(true);
                    else
                        info.ChangeAnimInfo(ECombatStateAnimInfo::ComboSaved, true);
                }
                else
                {
                    if (info.ChargeStage != -1 && info.CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed))
                    {
                        auto meleeStance = Cast<UMeleeCombatStance>(CurrentCombatStance);
                        bool hasStamina = Owner->GetAttributeComponent()->Stamina->CurrentValue > 0;

                        auto callBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnAttackEnded);

                        if (hasStamina && Owner->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Falling &&  meleeStance->CanDoShit(ESpecialType::Air))
                        { // uncharging in air
                            info.AttackInfo = &meleeStance->GetCurrentAttackInfo(ESpecialType::Air);

                            Owner->GetAttributeComponent()->Stamina->Consume(info.AttackInfo->StaminaCost);

                            info.ResetAnimInfo();
                            info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                                                        
                            Owner->StartAnimation(info.AttackInfo->AttackMontage, callBack, AttackSpeed);

                            return;
                        }
                        else if (hasStamina && currentAttack->AttackMontage && info.ChargeTime >= meleeStance->ComboAttacks[GetComboCount()].ChargeAttackStages[CombatFSM.currentState()->ChargeStage].ChargeTimeNeeded)
                        { // uncharging on ground
                            Owner->GetAttributeComponent()->Stamina->Consume(currentAttack->StaminaCost);

                            info.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, false);
                            info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                            
                            Owner->StartAnimation(currentAttack->AttackMontage, callBack, AttackSpeed, FAnimSectionNames::End);

                            return;
                        }

                        // charging cancelled
                        info.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, false);
                        info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);

                        Owner->StartAnimation(currentAttack->AttackMontage, callBack, AttackSpeed, FAnimSectionNames::Cancel);
                    }
                }
            }
        }
        else if (AttackPressed)// starting first attack
            UpdateCombatFSM();
    }
    else
    {
        if (CombatFSM.is(ECombatState::Attack))
        {
            auto& info = CombatFSM[ECombatState::Attack];

            if (info.CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed) && !AttackPressed)
            {  // check bow charging time and maybe fire
                auto stance = Cast<URangeCombatStance>(CurrentCombatStance); 
                check(stance && "Stance is invalid");
                auto callBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnAttackEnded);

                if (info.ChargeTime < ChargeStartTime)  // fire if drew enough, cancel otherwise TODO calculate min charge time for different bows
                    Owner->StartAnimation(stance->AttackInfo.AttackMontage, callBack, 1.f, FAnimSectionNames::Cancel);
                else
                    Owner->StartAnimation(stance->AttackInfo.AttackMontage, callBack, 1.f, FAnimSectionNames::End);
            }
        }
        else if (AttackPressed && !CombatFSM.is(ECombatState::Reload))
            UpdateCombatFSM();
    }
}

void UCombatComponent::OnBlock(bool Started) 
{
    if (!HasCombatStance())
        return;

    BlockPressed = Started;
    if (Owner->GetССComponent()->CheckCCMode(ECCState::LossOfBalance, static_cast<int>(ELossOfBalanceCCSubState::BlockedAttack)))
        return;
    if (!BlockPressed && CombatFSM.is(ECombatState::Block)) // end block
    {
        CombatFSM.currentState()->ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
        auto callBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnBlockEnded);
        Owner->StartAnimation(Owner->AnimInstance->GetMove(ECombatMovementType::Block), callBack, AttackSpeed, FAnimSectionNames::End);
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
}

void UCombatComponent::StartNextComboAttack(bool IsPauseAttack)
{
    check(CombatFSM.is(ECombatState::Attack) && "StartNextComboAttack: not in attack state");
    check(CurrentCombatStance && "StartNextComboAttack: doesn't have stance");
    auto meleeStance = Cast<UMeleeCombatStance>(CurrentCombatStance);
    if (CurrentCombo >= meleeStance->ComboAttacks.Num() - 1)
        return;
    if (Owner->GetAttributeComponent()->Stamina->CurrentValue <= 0)
        return;

    auto& stateInfo = *CombatFSM.currentState();
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
        Owner->GetAttributeComponent()->Stamina->Consume(nextAttackInfo->StaminaCost);
        
        IncreaseComboCount();
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
        auto callBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnAttackEnded);
        Owner->StartAnimation(stateInfo.AttackInfo->AttackMontage, callBack, AttackSpeed, stateInfo.AttackInfo->AnimSectionName);
    }
}

UItemWeaponRanged* UCombatComponent::GetEquippedRangedWeapon() const
{
    if (HasCombatStance() && !CurrentCombatStance->IsMelee())
        //if (auto RangedWeapon = Cast<UItemWeaponRanged>(CurrentCombatStance->GetArmsFromSlot(EArmsSlot::MainHand)))
        if (auto RangedWeapon = Cast<UItemWeaponRanged>(GetArmItem(EArmsSlot::MainHand)))
            return RangedWeapon;
    return nullptr;
}

void UCombatComponent::AddRangedWeaponSpread()
{
    if (auto Weapon = GetEquippedRangedWeapon())
    {
        if (GetWorld()->TimeSince(Weapon->GetLastAddSpreadTime()) > CameraMoveSpreadAddTimeOut)
            Weapon->AddSpread();
    }
}

void UCombatComponent::ToggleHandMode()
{
    if (IsBusy() || (HasCombatStance() && !CurrentCombatStance->IsMelee()))
        return;

    TwoHandMode = !TwoHandMode;
    UCombatStance* newStance = CheckStances(StanceStack, true);
    UCombatStance* defaultStance = CheckStances(Owner->AnimInstance->DefaultStances, true);
    if (newStance == nullptr)
    {
        if (defaultStance != nullptr)
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

UBowCombatStance *UCombatComponent::IsDrawingBow() const
{
    if (HasCombatStance() && CombatFSM.is(ECombatState::Attack))
        return Cast<UBowCombatStance>(GetCurrentCombatStance());
    return nullptr;
}

void UCombatComponent::OnWeaponSwitchEnded(UAnimMontage* animMontage, bool bInterrupted)
{
    auto& stateInfo = CombatFSM[ECombatState::WeaponSwitch];
    IsWeaponDrawn = !IsWeaponDrawn;
    if (!stateInfo.CheckAnimInfo(ECombatStateAnimInfo::WeaponGrabbed))
        ArmsSetup(IsWeaponDrawn);
        //CurrentCombatStance->ArmsSetup(IsWeaponDrawn);
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
    UpdateCombatFSM();
}

void UCombatComponent::OnBlockEnded(UAnimMontage* animMontage, bool bInterrupted)
{
    if (bInterrupted && CombatFSM.is(ECombatState::Parry))// interrupted by Parry
        return;
    auto& stateInfo = CombatFSM[ECombatState::Block];
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
    auto& stateInfo = CombatFSM[ECombatState::Parry];
    if (CombatFSM.isInState(ECombatState::Parry))
    {
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
        UpdateCombatFSM();
    }
}

void UCombatComponent::OnAttackEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    if (CombatFSM.currentState()->Id != ECombatState::Attack)  // interrupted
        return;
    auto& stateInfo = CombatFSM[ECombatState::Attack];
    if (stateInfo.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt))
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, false);  // continued combo
    else
    {
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
        UpdateCombatFSM();
    }
}

bool CheckHitActorCollision(const AItemActor* WeaponActor, const UPrimitiveComponent* HitComp)
{
    return (WeaponActor != nullptr) && (WeaponActor->ItemCollision == HitComp);
}

bool UCombatComponent::HitOwnMeleeWeapons(const UPrimitiveComponent* HitComp, const AActor* HitActor) 
{
    auto weaponActor = Cast<AItemActor>(HitActor);
    if (weaponActor)
        return weaponActor->ItemCollision == HitComp;

    return false;
}

bool UCombatComponent::CheckActorCollisionActive(const AItemActor* WeaponActor)
{
    return (WeaponActor != nullptr) && (WeaponActor->ItemCollision->GetGenerateOverlapEvents());
}

UItemArms* UCombatComponent::FindHitMeleeWeapon(const UPrimitiveComponent* HitComp) const
{
    //AItemActor* weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::MainHand);
    AItemActor* weaponActor = GetArmActor(EArmsSlot::MainHand);
    if (CheckHitActorCollision(weaponActor, HitComp))
        return Cast<UItemArms>(weaponActor->ItemBase);
    //weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::OffHand);
    weaponActor = GetArmActor(EArmsSlot::OffHand);
    if (CheckHitActorCollision(weaponActor, HitComp))
        return Cast<UItemArms>(weaponActor->ItemBase);
    //weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::RightFoot);
    weaponActor = GetArmActor(EArmsSlot::RightFoot);
    if (CheckHitActorCollision(weaponActor, HitComp))
        return Cast<UItemArms>(weaponActor->ItemBase);
    //weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::LeftFoot);
    weaponActor = GetArmActor(EArmsSlot::LeftFoot);
    if (CheckHitActorCollision(weaponActor, HitComp))
        return Cast<UItemArms>(weaponActor->ItemBase);

    return nullptr;
}

TArray<AItemActor*> UCombatComponent::FindActiveMeleeWeapons() const
{
    TArray<AItemActor*> result;
    if(!CurrentCombatStance)
        return result;

    //AItemActor* weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::MainHand);
    AItemActor* weaponActor = GetArmActor(EArmsSlot::MainHand);
    if (CheckActorCollisionActive(weaponActor))
        result.Push(weaponActor);
    //weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::OffHand);
    weaponActor = GetArmActor(EArmsSlot::OffHand);
    if (CheckActorCollisionActive(weaponActor))
        result.Push(weaponActor);
    //weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::RightFoot);
    weaponActor = GetArmActor(EArmsSlot::RightFoot);
    if (CheckActorCollisionActive(weaponActor))
        result.Push(weaponActor);
    //weaponActor = CurrentCombatStance->GetArmsActorFromSlot(EArmsSlot::LeftFoot);
    weaponActor = GetArmActor(EArmsSlot::LeftFoot);
    if (CheckActorCollisionActive(weaponActor))
        result.Push(weaponActor);

    return result;
}


void UCombatComponent::UpdateOwnerAnims(const UCombatStance* DefaultStance) 
{
    check(CurrentCombatStance && "Current Combat Stance is Invalid");
    check(DefaultStance && "Default Stance is Invalid");

    for(int i=0; i<static_cast<int>(ECombatMovementType::MAX);++i)
        Owner->AnimInstance->CombatMoves[i] = CurrentCombatStance->CombatMoves[i] ? CurrentCombatStance->CombatMoves[i] : DefaultStance->CombatMoves[i];

    for(int i=0; i<static_cast<int>(ECombatMotion::MAX);++i)
        Owner->AnimInstance->CombatMotions[i] = CurrentCombatStance->CombatMotions[i] ? CurrentCombatStance->CombatMotions[i] : DefaultStance->CombatMotions[i];
    
    for(int i=0; i<static_cast<int>(ELossOfBalanceCCSubState::MAX);++i)
        Owner->AnimInstance->CCLOBAnims[i] = CurrentCombatStance->CCLOBAnims[i] ? CurrentCombatStance->CCLOBAnims[i] : DefaultStance->CCLOBAnims[i];
}

bool UCombatComponent::IsRangedDraw(float &DrawRatio) const
{
    auto Ranged = Cast<UBowCombatStance>(CurrentCombatStance);
    if(!Ranged) return false;    
    DrawRatio = CombatFSM.currentState()->ChargeTime / GetMaxDrawTime(Ranged);
    return true;
}

float UCombatComponent::GetMaxDrawTime(UBowCombatStance *Stance) const 
{ 
    //AItemActor* weaponActor = Stance->GetArmsActorFromSlot(EArmsSlot::MainHand);
    AItemActor* weaponActor = GetArmActor(EArmsSlot::MainHand);
    check(weaponActor);
    auto weaponItem = Cast<UItemWeaponRanged>(weaponActor->ItemBase);
    check(weaponItem);
    check(Stance->BowDrawingAnim);

    EAttributes modAttribute = weaponItem->WeaponType == EWeaponType::Longbow ? EAttributes::Str : EAttributes::Dex;    
    float animDuration = Stance->BowDrawingAnim->GetPlayLength();
    float animRateModifier = Owner->GetAttributeComponent()->GetAnimRateFromModifier(modAttribute);
    float playRate = animDuration / (weaponItem->MaxTensionStrength / animRateModifier) * DrawTimeModFromTension;
    return animDuration / playRate;
}

float UCombatComponent::GetLastDrawRatio() const 
{
    auto BowStance = Cast<UBowCombatStance>(CurrentCombatStance);
    return BowStance ? LastChargeTime / GetMaxDrawTime(BowStance) : 1.f;
}

float UCombatComponent::CalculateAmmoInitialSpeed(UItemWeaponRanged* Weapon) const
{
    return Weapon->MaxTensionStrength * Weapon->TensionCoefficient * GetLastDrawRatio();
}

float UCombatComponent::GetCurrentDrawRatio() const
{
    auto BowStance = Cast<UBowCombatStance>(CurrentCombatStance);
    return BowStance ? CombatFSM.currentState()->ChargeTime / GetMaxDrawTime(BowStance) : 1.f;
}

void UCombatComponent::SetNewStance(UCombatStance* OldStance, UCombatStance* NewStance, const UCombatStance* DefaultStance)
{
    bool isSwitchPending = PendingStance.IsSet();
    if (OldStance)
    {
        OldStance->StanceUnSet();
    }

    CurrentCombatStance = NewStance;

    if (NewStance)
    {
        TwoHandMode = NewStance->StanceType == EWeaponCombination::TwoHand ? true : false;

        UpdateOwnerAnims(DefaultStance);
        NewStance->StanceSet(Owner);
        //if(WeaponDrawn())
            //NewStance->ArmsSetup(false); // attach the weapons

        ResetComboCount();        
        WeaponsMap.Empty();
        CurrentCombatStance->AllocateWeapons(Owner, WeaponsMap);

        //ArmsSetup(false);       

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

UCombatStance* UCombatComponent::CheckStances(const TArray<UCombatStance*>& Stances, bool CheckHandMode)
{
    for (auto stance : Stances)
    {
        if (DoesStanceFit(stance, CheckHandMode))
            return stance;
    }
    return nullptr;
}

bool UCombatComponent::DoesStanceFit(UCombatStance* Stance, bool CheckHandMode) const
{
    if (CurrentCombatStance == Stance)
        return false;

    //Stance->SetWeaponVars(Owner);

    bool correctHandMode = true;
    if (CheckHandMode)
    {
        correctHandMode =  TwoHandMode ? (Stance->StanceType == EWeaponCombination::TwoHand) : 
                                         (Stance->StanceType != EWeaponCombination::TwoHand);
    }
    bool result = IsStanceValid(Stance) && Stance->CheckRequirement(Owner) && correctHandMode;

    //TODO: Remove Leak
    //CurrentWeapons.Empty();

    //if (!result) Stance->ResetWeaponVars();
    return result;
}

FStanceResolveResult UCombatComponent::ResolveNewStance(UCombatStance* NewStance, bool WeaponChanged)
{
    UCombatStance* DefaultStance = CheckStances(Owner->AnimInstance->DefaultStances);
        
    if(!NewStance) //Если стойка не валидна, ищем в стеке
        NewStance = CheckStances(StanceStack);    

    if(!NewStance) //Если в стеке не нашли
    {
        if(!DefaultStance) // И если нет дефолтного, то возвращаемся 
            if(!WeaponChanged) 
                return FStanceResolveResult{true, false};
            else
                return FStanceResolveResult{false, false};
        else
            NewStance = DefaultStance; //Ставим дефолт
    }       

    if(!DoesStanceFit(NewStance))
        return FStanceResolveResult();

    if(NewStance == CurrentCombatStance)
    {
        //TODO: DO Something, but i don't remember
        return FStanceResolveResult();
    }

    if(CurrentCombatStance)
    {
        if (NewStance->GetClass() == CurrentCombatStance->GetClass()) // Если стойки одинакового класса
            return FStanceResolveResult{false, false}; //То ничего не делаем
    }

    StanceChanged(CurrentCombatStance, NewStance, DefaultStance, WeaponChanged);
    return FStanceResolveResult{false, true}; 
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

void UCombatComponent::CalculateMaxCombatSpeeds()
{
    float speedMod = Owner->GetMovementComponent()->GetSpeedModifier();
    Owner->GetMovementComponent()->MaxRunSpeed = CurrentCombatStance->MaxCombatRunSpeed * speedMod;
    Owner->GetMovementComponent()->MaxSprintSpeed = CurrentCombatStance->MaxCombatSprintSpeed * speedMod;
}

void UCombatComponent::CalculateMaxBlockingSpeed(float SpeedModifier)
{
    Owner->GetMovementComponent()->MaxRunSpeed = CurrentCombatStance->MaxBlockOrActionSpeed * SpeedModifier;
}

bool UCombatComponent::CanBeInterruptedWithBlock(int CurrentState) const
{
    if (CurrentCombatStance->IsMelee())
    {
        return !CombatFSM[CurrentState].CheckAnimInfo(ECombatStateAnimInfo::CollisionOn) && BlockPressed;
    }
    else
        return true;
}

void UCombatComponent::WeaponSetup(UItemArms* Weapon, AItemActor* Actor, bool Draw, FName Slot, bool MainHand)
{
    if (Slot.IsNone())
        Slot = GetSlotFromWeaponType(Weapon->WeaponType, Draw, MainHand);

    if (CurrentCombatStance->IsMelee())
    {
        if (Draw)
            Actor->ItemCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &UCombatComponent::OnWeaponOverlap);
        else
            Actor->ItemCollision->OnComponentBeginOverlap.RemoveDynamic(this, &UCombatComponent::OnWeaponOverlap);
    }
    else
    {
        if (Draw)
            Actor->ItemCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &UCombatComponent::OnRangedWeaponOverlap);
        else
            Actor->ItemCollision->OnComponentBeginOverlap.RemoveDynamic(this, &UCombatComponent::OnRangedWeaponOverlap);
    }

    SetWeaponCollision(Actor, false); // disable equipped weapon collision
    UEquipmentComponent::AttachItemActor(Owner, Actor, Slot, Draw ? FItemActorEquipmentSlotNames::WeaponEquipSocket : FItemActorEquipmentSlotNames::EquipSocket);
}

void UCombatComponent::SetWeaponCollision(AItemActor* WeaponActor, bool Enable)
{
    WeaponActor->ItemCollision->SetGenerateOverlapEvents(Enable);
}

ESpecialType UCombatComponent::GetSpecialType(bool &isDirectional) const 
{
    isDirectional = false;
    if (Owner->GetMovementComponent()->bIsSprinting)
        return ESpecialType::Sprint;
    else if (Owner->GetMovementComponent()->IsCrouching())
        return ESpecialType::Crouch;
    else if (Owner->GetMovementComponent()->IsFalling())
        return ESpecialType::Air;
    else
    {
        isDirectional = true;
        if (SpecialDirections & ESpecialDirection::SFORWARD)
            return ESpecialType::Forward;
        else if (SpecialDirections & ESpecialDirection::SBACK)
            return ESpecialType::Back;
        else if (SpecialDirections & ESpecialDirection::SLEFT)
            return ESpecialType::Left;
        else if (SpecialDirections & ESpecialDirection::SRIGHT)
            return ESpecialType::Right;
    }

    return ESpecialType::None; 
}


bool UCombatComponent::IdleToBlock() const
{
    bool hasStamina = Owner->GetAttributeComponent()->Stamina->CurrentValue > 0;
    bool hasAnim = Owner->AnimInstance->GetMove(ECombatMovementType::Block) != nullptr;
    bool IsBlockAllowed = CurrentCombatStance ? CurrentCombatStance->IsBlockAllowed : false; 
    return hasAnim && IsBlockAllowed && WeaponDrawn() && IsBlockPressed() && hasStamina;
}

bool UCombatComponent::IdleToAttack() const
{
    bool hasStamina = Owner->GetAttributeComponent()->Stamina->CurrentValue > 0;
    bool isDirectional;
    ESpecialType special = GetSpecialType(isDirectional);    
    bool dontknowhowtonameit = CurrentCombatStance->CanDoShit(special);
    if (!WeaponDrawn() || !IsAttackPressed() || !hasStamina || Owner->GetMovementComponent()->IsTransitionInProgress() || !dontknowhowtonameit)
        return false;
    return true;
}

bool UCombatComponent::IdleToWeaponSwitch() const
{
    return IsWeaponSwitchPressed() || PendingStance.IsSet();
}

bool UCombatComponent::IdleToReload() const
{
    if (CurrentCombatStance->IsMelee())
        return false;

    bool hasStamina = Owner->GetAttributeComponent()->Stamina->CurrentValue > 0;
    auto AmmoItem = Owner->GetEquipmentComponent()->GetEquipmentFromSlot(EItemSlot::AmmoSlot).Item;
    bool hasAmmo = AmmoItem && AmmoItem->StackCount;

    return CurrentCombatStance->CombatMoves[static_cast<int>(ECombatMovementType::Reload)] /*Ranged->ReloadAnim*/ && !IsWeaponLoaded && hasStamina && hasAmmo && IsAttackPressed() && WeaponDrawn();
}

bool UCombatComponent::AttackToBlock() const 
{   
    bool checkEnd = CombatFSM[ECombatState::Attack].CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && 
                    !CombatFSM[ECombatState::Attack].CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt) &&
                    IsBlockPressed();
    //bool hasAnim = Owner->AnimInstance->CurrentBlockAnim != nullptr;
    bool hasAnim = Owner->AnimInstance->GetMove(ECombatMovementType::Block) != nullptr;
    bool IsBlockAllowed = CurrentCombatStance ? CurrentCombatStance->IsBlockAllowed : false;

    return hasAnim && IsBlockAllowed && (checkEnd || CanBeInterruptedWithBlock(ECombatState::Attack));
}

bool UCombatComponent::AttackToIdle() const
{
    return CombatFSM[ECombatState::Attack].CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && !IsBlockPressed();
}

bool UCombatComponent::WeaponSwitchToIdle() const
{
    return CombatFSM[ECombatState::WeaponSwitch].CheckAnimInfo(ECombatStateAnimInfo::AnimEnded);
}

bool UCombatComponent::BlockToIdle() const
{
    return CombatFSM[ECombatState::Block].CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) &&
           !CombatFSM[ECombatState::Block].CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt);
}

bool UCombatComponent::BlockToParry() const
{
    const bool hasStamina = Owner->GetAttributeComponent()->Stamina->CurrentValue > 0;
    const bool isInfo = CombatFSM[ECombatState::Block].CheckAnimInfo(ECombatStateAnimInfo::BlockOrParryActive);
    //const bool hasAnim = Owner->AnimInstance->CurrentParryAnim != nullptr;
    const bool hasAnim = Owner->AnimInstance->GetMove(ECombatMovementType::Parry) != nullptr;
    const bool isNotFalling = !Owner->GetMovementComponent()->IsFalling();
    const bool IsParryAllowed = CurrentCombatStance->IsParryAllowed;

    return hasAnim && IsParryAllowed && isNotFalling && isInfo && IsAttackPressed() && hasStamina;
}

bool UCombatComponent::ParryToBlock() const 
{
    bool hasStamina = Owner->GetAttributeComponent()->Stamina->CurrentValue > 0; // can't start block without stamina
    bool checkEnd = CombatFSM[ECombatState::Parry].CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && IsBlockPressed();
    //bool hasAnim = Owner->AnimInstance->CurrentBlockAnim != nullptr;
    const bool hasAnim = Owner->AnimInstance->GetMove(ECombatMovementType::Block) != nullptr;
    bool IsBlockAllowed = CurrentCombatStance ? CurrentCombatStance->IsBlockAllowed : false;

    return hasAnim && IsBlockAllowed && hasStamina && checkEnd && !Owner->GetMovementComponent()->IsRootOrTeleport();
}

bool UCombatComponent::ParryToIdle() const
{
    return CombatFSM[ECombatState::Parry].CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && !IsBlockPressed();
}

bool UCombatComponent::ReloadToAttack() const
{
    bool hasAnim = CurrentCombatStance->GetCurrentAttackInfo().AttackMontage != nullptr;
    bool hasStamina = Owner->GetAttributeComponent()->Stamina->CurrentValue > 0;
    bool IsTransition = Owner->GetMovementComponent()->IsTransitionInProgress();

    return hasAnim && WeaponDrawn() && IsAttackPressed() && IsWeaponLoaded && IsWeaponLoaded && hasStamina && !IsTransition;
}

bool UCombatComponent::ReloadToIdle() const
{
    return CombatFSM[ECombatState::Reload].CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && !IsAttackPressed();
}

void UCombatComponent::OnIdleEnter() 
{
    ResetRotationRate();
    if (PendingStance.IsSet())
        SetNewStance(CurrentCombatStance, PendingStance.NewStance, PendingStance.DefaultStance);
}

void UCombatComponent::OnIdleLeave() 
{
    CombatFSM[ECombatState::Idle].ResetAnimInfo();
    ResetRotationRate();
}

void UCombatComponent::OnAbilityEnter()
{
    if (!WeaponDrawn())// not in combat mode
    {
        Owner->GetMovementComponent()->maxSpeedBlock = true;
        Owner->GetMovementComponent()->bUseControllerDesiredRotation = true;
        Owner->GetMovementComponent()->bOrientRotationToMovement = false;
    }
    Owner->GetAbilityManager()->CalculateMaxCastingSpeed(Owner->GetMovementComponent()->GetSpeedModifier());
    Owner->GetAttributeComponent()->Stamina->Consume(Owner->GetAbilityManager()->CurrentAbility->StaminaCost);
    Owner->GetAttributeComponent()->Stamina->BlockRegen();
    Owner->GetMovementComponent()->MoveAbilitiesCurrent = Owner->GetAbilityManager()->CurrentAbility->MoveAbilities;
    Owner->GetAbilityManager()->ActivateAbility();
}

void UCombatComponent::OnAbilityLeave()
{
    if (WeaponDrawn())// in combat mode
        CalculateMaxCombatSpeeds();
    else
    {
        Owner->GetMovementComponent()->maxSpeedBlock = false;
        Owner->GetMovementComponent()->CalculateMaxSpeeds(Owner->GetMovementComponent()->GetSpeedModifier());
        Owner->GetMovementComponent()->bUseControllerDesiredRotation = false;
        Owner->GetMovementComponent()->bOrientRotationToMovement = true;
    }
    Owner->GetAttributeComponent()->Stamina->UnblockRegen();
    Owner->GetMovementComponent()->MoveAbilitiesCurrent = Owner->GetMovementComponent()->MoveAbilitiesDefault; // TODO think how to restore it to last state
    Owner->GetAbilityManager()->SetGlobalCooldown();
    Owner->ClearAbilityCameraMode();
}

void UCombatComponent::OnAttackEnter() 
{
    auto callBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnAttackEnded);
    if (CurrentCombatStance->IsMelee())
    {
        bool isDirectional;
        ESpecialType special = GetSpecialType(isDirectional);

        FAttackInfo currentAttack;
        if (special != ESpecialType::None && CurrentCombatStance->CanDoShit(special))
        {
            currentAttack = CurrentCombatStance->GetCurrentAttackInfo(special);
            CombatFSM[ECombatState::Attack].AttackInfo = &CurrentCombatStance->GetCurrentAttackInfo(special);
        }
        else
        {
            currentAttack = CurrentCombatStance->GetCurrentAttackInfo();
            CombatFSM[ECombatState::Attack].AttackInfo = &CurrentCombatStance->GetCurrentAttackInfo();
        }

        bool isComboAttack = currentAttack.Special == ESpecialType::None;
        bool hasChargeAttack = CurrentCombatStance->HasChargeAttack();
        Owner->StartAnimation(currentAttack.AttackMontage, callBack, AttackSpeed, currentAttack.AnimSectionName);

        if (isComboAttack && hasChargeAttack)
            CombatFSM[ECombatState::Attack].ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, true);
        Owner->GetAttributeComponent()->Stamina->Consume(currentAttack.StaminaCost);
        Owner->GetMovementComponent()->RotationRate.Yaw = ActionRotationRate;
    }
    else
    {
        float playRate = 1.f;
        bool IsBowStance = Cast<UBowCombatStance>(CurrentCombatStance) != nullptr;
        if (IsBowStance)
        {
            // calculate speed from anim length and bow max tension curve time =  animLength / chargeLength
            // where chargeLength = mod * MaxTensionStrength / animRateModifier
            // add load anim sequence into stance to get length from
            //AItemActor* weaponActor = GetCurrentCombatStance()->GetArmsActorFromSlot(EArmsSlot::MainHand);
            AItemActor* weaponActor = GetArmActor(EArmsSlot::MainHand);
            auto Ranged = Cast<UBowCombatStance>(CurrentCombatStance);
            check(Ranged);
            check(weaponActor);
            auto weaponItem = Cast<UItemWeaponRanged>(weaponActor->ItemBase);
            check(weaponItem);
            EAttributes modAttribute = weaponItem->WeaponType == EWeaponType::Longbow ? EAttributes::Str : EAttributes::Dex;

            check(Ranged->BowDrawingAnim);
            float animDuration = Ranged->BowDrawingAnim->GetPlayLength();  // in seconds
            float animRateModifier = Owner->GetAttributeComponent()->GetAnimRateFromModifier(modAttribute);
            playRate = animDuration / (weaponItem->MaxTensionStrength / animRateModifier) * DrawTimeModFromTension;
            if (auto actorMesh = Cast<USkeletalMeshComponent>(weaponActor->ItemMesh))
                if (auto animInstance = Cast<URangedWeaponAnimInstance>(actorMesh->GetAnimInstance()))
                {
                    animInstance->IsLoading = true;
                    animInstance->LoadingPlayRate = playRate;  // TODO calculate playrate based on bow draw anim, same for crossbows reload
                }
            CombatFSM[ECombatState::Attack].ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, true);
            LastChargeTimeStaminaConsume = 0;
            LastChargeTime = 0;
        }
        auto AttackInfo = CurrentCombatStance->GetCurrentAttackInfo();
        Owner->StartAnimation(AttackInfo.AttackMontage, callBack, playRate, AttackInfo.AnimSectionName);
        Owner->GetAttributeComponent()->Stamina->Consume(AttackInfo.StaminaCost);
    }
    Owner->GetAttributeComponent()->Stamina->BlockRegen();
    CalculateMaxBlockingSpeed(Owner->GetMovementComponent()->GetSpeedModifier());
}

void UCombatComponent::OnAttackLeave() 
{
    if (CurrentCombatStance->IsMelee())
    {
        CombatFSM[ECombatState::Attack].AttackInfo = nullptr;
        CombatFSM[ECombatState::Attack].ChargeTime = 0.f;
        CombatFSM[ECombatState::Attack].ChargeStage = -1;
        ResetComboCount();
        CharactersHit.Reset();
        auto activeWeapons = FindActiveMeleeWeapons();
        for (auto& weaponActor : activeWeapons)
            UCombatComponent::SetWeaponCollision(weaponActor, false);
    }
    else
    {
        auto BowStance = Cast<UBowCombatStance>(CurrentCombatStance);
        if (BowStance)
        {
            LastChargeTime = CombatFSM[ECombatState::Attack].ChargeTime;
            CombatFSM[ECombatState::Attack].ChargeTime = 0.f;
            AItemActor* weaponActor = GetArmActor(EArmsSlot::MainHand);
            check(weaponActor);
            if (auto actorMesh = Cast<USkeletalMeshComponent>(weaponActor->ItemMesh))
                if (auto animInstance = Cast<URangedWeaponAnimInstance>(actorMesh->GetAnimInstance()))
                    animInstance->IsLoaded = false;
        }
    }
    ResetActionState(ECombatState::Attack);
    CalculateMaxCombatSpeeds();
}

void UCombatComponent::OnSwitchEnter() 
{
    const auto Anim = WeaponDrawn() ? Owner->AnimInstance->GetMove(ECombatMovementType::Sheath) : Owner->AnimInstance->GetMove(ECombatMovementType::Draw);
    auto CallBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnWeaponSwitchEnded);            
    const auto AnimRate = Owner->GetAttributeComponent()->GetAnimRateFromModifier(EAttributes::Dex);
    Owner->StartAnimation(Anim, CallBack, AnimRate);
}

void UCombatComponent::OnSwitchLeave() 
{
    if (WeaponDrawn()) // in combat mode
    {
        Owner->GetMovementComponent()->maxSpeedBlock = true;
        CalculateMaxCombatSpeeds();
        Owner->GetMovementComponent()->bUseControllerDesiredRotation = true;
        Owner->GetMovementComponent()->bOrientRotationToMovement = false;
        Owner->GetMovementComponent()->RotationRate.Yaw = CombatModeRotationRate;
        if (PendingStance.IsSet())
            PendingStance.Reset();
    }
    else // exit combat mode
    {// reset max speeds
        Owner->GetMovementComponent()->maxSpeedBlock = false;
        Owner->GetMovementComponent()->CalculateMaxSpeeds(Owner->GetMovementComponent()->GetSpeedModifier());
        Owner->GetMovementComponent()->bUseControllerDesiredRotation = false;
        Owner->GetMovementComponent()->bOrientRotationToMovement = true;
    }
    CombatFSM[ECombatState::WeaponSwitch].ResetAnimInfo();
}

void UCombatComponent::OnBlockEnter() 
{
    CalculateMaxBlockingSpeed(Owner->GetMovementComponent()->GetSpeedModifier());
    const auto Anim = Owner->AnimInstance->GetMove(ECombatMovementType::Block);        
    auto CallBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnBlockEnded);
    Owner->GetAttributeComponent()->Stamina->RegenCoef += BlockStaminaRegenRateMod; // slowing down stamina regen rate
    Owner->StartAnimation(Anim, CallBack, AttackSpeed, FAnimSectionNames::Start);
}

void UCombatComponent::OnBlockLeave() 
{
    CalculateMaxCombatSpeeds();
    Owner->GetAttributeComponent()->Stamina->RegenCoef -= BlockStaminaRegenRateMod; // reseting stamina regen rate
    CombatFSM[ECombatState::Block].ResetAnimInfo();
}

void UCombatComponent::OnParryEnter() 
{
    auto ParryStaminaCost = CurrentCombatStance->ParryStaminaCost;
    auto callBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnParryEnded);
    const auto Anim = Owner->AnimInstance->GetMove(ECombatMovementType::Parry);
    auto Rate = CurrentCombatStance->StanceType == EWeaponCombination::Ranged ? Owner->GetAttributeComponent()->GetAnimRateFromModifier(EAttributes::Dex) : AttackSpeed;
    Owner->StartAnimation(Anim, callBack, Rate);
    Owner->GetAttributeComponent()->Stamina->Consume(ParryStaminaCost);
    Owner->GetAttributeComponent()->Stamina->BlockRegen();
    Owner->GetMovementComponent()->RotationRate.Yaw = ActionRotationRate;
}

void UCombatComponent::OnReloadEnded(UAnimMontage* AnimMontage, bool bInterrupted)
{    
    auto& stateInfo = CombatFSM[ECombatState::Reload];
    auto Ranged = Cast<URangeCombatStance>(CurrentCombatStance);

    if (!bInterrupted || stateInfo.CheckAnimInfo(ECombatStateAnimInfo::AmmoGrabbed))
    {
        if (!stateInfo.CheckAnimInfo(ECombatStateAnimInfo::AmmoAttach))
            Ranged->AttachAmmoActorToWeapon();
        IsWeaponLoaded = true;
    }
    else
    {
        AItemActor* weaponActor = GetArmActor(EArmsSlot::MainHand);
        if (auto actorMesh = Cast<USkeletalMeshComponent>(weaponActor->ItemMesh))
            if (auto animInstance = Cast<URangedWeaponAnimInstance>(actorMesh->GetAnimInstance()))
            {
                animInstance->IsLoading = false;
                animInstance->IsLoaded = false;
            }
    }

    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
    UpdateCombatFSM();
}

void UCombatComponent::OnParryLeave() 
{
    ResetActionState(ECombatState::Parry);
    if (!CurrentCombatStance->IsMelee())
    {
        CharactersHit.Reset();
        auto activeWeapons = FindActiveMeleeWeapons();
        for (auto& weaponActor : activeWeapons)
            UCombatComponent::SetWeaponCollision(weaponActor, false);
    }
}

void UCombatComponent::OnReloadEnter()
{
    auto Ranged = Cast<URangeCombatStance>(CurrentCombatStance);
    auto AnimRate = Owner->GetAttributeComponent()->GetAnimRateFromModifier(Ranged->AnimRateMod);
    auto callBack = FOnMontageBlendingOutStarted::CreateUObject(this, &UCombatComponent::OnReloadEnded);
    Owner->StartAnimation(CurrentCombatStance->CombatMoves[static_cast<int>(ECombatMovementType::Reload)], callBack, AnimRate);

    auto CrossbowStance = Cast<UCrossbowCombatStance>(CurrentCombatStance);
    if (CrossbowStance)
    {
        Owner->GetAttributeComponent()->Stamina->Consume(CrossbowStance->ReloadStaminaCost);
        Owner->GetAttributeComponent()->Stamina->BlockRegen();
        AItemActor* weaponActor = GetArmActor(EArmsSlot::MainHand);
        if (auto actorMesh = Cast<USkeletalMeshComponent>(weaponActor->ItemMesh))
            if (auto weaponAnimInstance = Cast<URangedWeaponAnimInstance>(actorMesh->GetAnimInstance()))
            {
                weaponAnimInstance->IsLoading = true;
                weaponAnimInstance->LoadingPlayRate = AnimRate;
            }
    }
}

void UCombatComponent::OnReloadLeave()
{
    CombatFSM[ECombatState::Reload].ResetAnimInfo();
    auto CrossbowStance = Cast<UCrossbowCombatStance>(CurrentCombatStance);
    if (CrossbowStance)
        Owner->GetAttributeComponent()->Stamina->UnblockRegen();
}

void UCombatComponent::InitFSM()
{
    CombatFSM.add(ECombatState::Idle)
        .add(ECombatState::WeaponSwitch)
        .add(ECombatState::Attack)
        .add(ECombatState::Block)
        .add(ECombatState::Parry)
        .add(ECombatState::Reload);

    CombatFSM.setCurrentState(ECombatState::Idle);

    CombatFSM.link(ECombatState::Idle, ECombatState::WeaponSwitch, FLinkReq::CreateUObject(this, &UCombatComponent::IdleToWeaponSwitch));
    CombatFSM.link(ECombatState::Idle, ECombatState::Block, FLinkReq::CreateUObject(this, &UCombatComponent::IdleToBlock));
    CombatFSM.link(ECombatState::Idle, ECombatState::Attack, FLinkReq::CreateUObject(this,&UCombatComponent::IdleToAttack));     
    CombatFSM.link(ECombatState::Idle, ECombatState::Reload, FLinkReq::CreateUObject(this, &UCombatComponent::IdleToReload));
    CombatFSM.link(ECombatState::Block, ECombatState::Idle, FLinkReq::CreateUObject(this, &UCombatComponent::BlockToIdle));
    CombatFSM.link(ECombatState::Block, ECombatState::Parry, FLinkReq::CreateUObject(this, &UCombatComponent::BlockToParry));
    CombatFSM.link(ECombatState::Attack, ECombatState::Idle, FLinkReq::CreateUObject(this, &UCombatComponent::AttackToIdle));
    CombatFSM.link(ECombatState::Attack, ECombatState::Block, FLinkReq::CreateUObject(this, &UCombatComponent::AttackToBlock));
    CombatFSM.link(ECombatState::Parry, ECombatState::Idle, FLinkReq::CreateUObject(this, &UCombatComponent::ParryToIdle));
    CombatFSM.link(ECombatState::Parry, ECombatState::Block, FLinkReq::CreateUObject(this, &UCombatComponent::ParryToBlock));
    CombatFSM.link(ECombatState::WeaponSwitch, ECombatState::Idle, FLinkReq::CreateUObject(this, &UCombatComponent::WeaponSwitchToIdle));
    CombatFSM.link(ECombatState::Reload, ECombatState::Attack, FLinkReq::CreateUObject(this, &UCombatComponent::ReloadToAttack));
    CombatFSM.link(ECombatState::Reload, ECombatState::Idle, FLinkReq::CreateUObject(this, &UCombatComponent::ReloadToIdle));

    CombatFSM[ECombatState::Idle].OnEnter.BindUObject(this, &UCombatComponent::OnIdleEnter);
    CombatFSM[ECombatState::Idle].OnLeave.BindUObject(this, &UCombatComponent::OnIdleLeave);

    CombatFSM[ECombatState::WeaponSwitch].OnEnter.BindUObject(this, &UCombatComponent::OnSwitchEnter);
    CombatFSM[ECombatState::WeaponSwitch].OnLeave.BindUObject(this, &UCombatComponent::OnSwitchLeave);

    CombatFSM[ECombatState::Block].OnEnter.BindUObject(this, &UCombatComponent::OnBlockEnter);
    CombatFSM[ECombatState::Block].OnLeave.BindUObject(this, &UCombatComponent::OnBlockLeave);

    CombatFSM[ECombatState::Parry].OnEnter.BindUObject(this, &UCombatComponent::OnParryEnter);    
    CombatFSM[ECombatState::Parry].OnLeave.BindUObject(this, &UCombatComponent::OnParryLeave);

    CombatFSM[ECombatState::Attack].OnEnter.BindUObject(this, &UCombatComponent::OnAttackEnter);
    CombatFSM[ECombatState::Attack].OnLeave.BindUObject(this, &UCombatComponent::OnAttackLeave);

    CombatFSM[ECombatState::Reload].OnEnter.BindUObject(this, &UCombatComponent::OnReloadEnter);
    CombatFSM[ECombatState::Reload].OnLeave.BindUObject(this, &UCombatComponent::OnReloadLeave);
}

void UCombatComponent::ResetComboCount()
{
    CurrentCombo = 0; 
}

int UCombatComponent::GetComboCount() const
{
    return CurrentCombo; 
}

void UCombatComponent::IncreaseComboCount()
{
    ++CurrentCombo; 
}

AItemActor* UCombatComponent::GetArmActor(EArmsSlot Slot) const
{
    auto it = WeaponsMap.Find(Slot);
    return it ? it->ItemActor.Get() : nullptr;
}

UItemArms* UCombatComponent::GetArmItem(EArmsSlot Slot) const 
{
    auto it = WeaponsMap.Find(Slot);
    auto actor = it ? it->ItemActor.Get() : nullptr;
    
    return actor ? Cast<UItemArms>(actor->ItemBase) : nullptr;
}

void UCombatComponent::ArmsSetup(bool Draw)
{
    for(auto arm : WeaponsMap)
    {
        check(arm.Value.ItemActor && "Weapon invalid");

        bool isMainHand = (arm.Key == EArmsSlot::MainHand);        
        WeaponSetup(Cast<UItemArms>(arm.Value.ItemActor->ItemBase), arm.Value.ItemActor.Get(), Draw, FName(), isMainHand);        
    }
}

void UCombatComponent::FireLoadedProjectile()
{
    if (HasCombatStance())
        if (auto RangedStance = Cast<URangeCombatStance>(CurrentCombatStance))
            RangedStance->FireLoadedAmmo();
}

bool UCombatComponent::IsStanceValid(UCombatStance *Stance) const
{
    if(!Stance) return false;

    auto MainHand = Owner->GetEquipmentComponent()->GetEquipmentFromSlot(EItemSlot::MainWeaponSlot).Item;
    auto OffHand = Owner->GetEquipmentComponent()->GetEquipmentFromSlot(EItemSlot::OffMeaponSlot).Item;
    
    if (Stance->IsUsingDefaultWeapons)
    {
        if (MainHand || OffHand)
            return false; // don't set unarmed stance if armed
        return true;
    }   

    const auto StanceType = Stance->StanceType;
    switch(StanceType)
    {
    case EWeaponCombination::Shield: 
    {
        if(!MainHand || !OffHand) 
            return false;

        auto MainItem = Cast<UItemArms>(MainHand->ItemBase);
        auto OffItem = Cast<UItemArms>(OffHand->ItemBase);
        check(MainItem && "Invalid Item");
        check(OffItem && "Invalid Item");

        return OffItem->WeaponType == EWeaponType::Shield && 
               Stance->AllowedWeaponTypes.Contains(MainItem->WeaponType) && 
               Stance->AllowedWeaponTypes.Contains(OffItem->WeaponType);
    }
    case EWeaponCombination::TwoWeapon: 
    {
        if(!MainHand || !OffHand) 
            return false;
        auto MainItem = Cast<UItemArms>(MainHand->ItemBase);
        auto OffItem = Cast<UItemArms>(OffHand->ItemBase);
        check(MainItem && "Invalid Item");
        check(OffItem && "Invalid Item");
        return Stance->AllowedWeaponTypes.Contains(MainItem->WeaponType) && 
               Stance->AllowedWeaponTypes.Contains(OffItem->WeaponType);
    }
    case EWeaponCombination::TwoHand:
    case EWeaponCombination::OneHand: 
    {
        bool hasOffhandEquipped = Stance->IsUsingDefaultWeapons ? false : (OffHand != nullptr);

        if(!MainHand || hasOffhandEquipped)
            return false;

        auto MainItem = Cast<UItemArms>(MainHand->ItemBase);
        check(MainItem && "Invalid Item");

        return Stance->AllowedWeaponTypes.Contains(MainItem->WeaponType);
    }
    case EWeaponCombination::Ranged:
    {
        auto MainItem = Cast<UItemArms>(MainHand->ItemBase);
        if(!MainHand) 
            return false;
        return MainHand && !OffHand && Stance->AllowedWeaponTypes.Contains(MainItem->WeaponType);
    }
    default:
        return false;
    }
}

