#include "CharacterCreature.h"
#include "CharacterAnimInstance.h"
#include "Components/CharacterMeshComponent.h"


ACharacterCreature::ACharacterCreature(const FObjectInitializer &ObjectInitializer) :
    Super(ObjectInitializer
        .SetDefaultSubobjectClass<UHumanoidMovementComponent>(ACharacter::CharacterMovementComponentName)
        .SetDefaultSubobjectClass<UCharacterMeshComponent>(ACharacter::MeshComponentName))
{
    PrimaryActorTick.bCanEverTick = true;
    CombatComponent = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
    EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>("EquipmentComponent");
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");
    AbilityManager = CreateDefaultSubobject<UAbilityManagerComponent>("AbilityComponent");
    CCComponent = CreateDefaultSubobject<UCCComponent>("CCComponent");
    CCInterruptDelegates.Emplace(ECCState::LossOfBalance, &ACharacterCreature::CCLOBEnded);
    CCInterruptDelegates.Emplace(ECCState::Stun, &ACharacterCreature::CCStunEnded);
    CCInterruptDelegates.Emplace(ECCState::KnockDown, &ACharacterCreature::CCKnockDownEnded);
    // TODO add other callbacks
    GetMovementComponent()->OnMoveModeChanged.AddDynamic(CombatComponent, &UCombatComponent::OnMovementModeChanged);
    EquipmentComponent->EquipmentSlotUpdated.AddDynamic(CombatComponent, &UCombatComponent::OnEquipmentUpdate);
    EquipmentComponent->EquipmentSwaped.AddDynamic(CombatComponent, &UCombatComponent::OnEquipmentSwap);
    KnockDownBaseTime = 5.f;
    KnockDownModBase = 0.1f;
    KnockDownWeightModBase = 1.f;
    MaxWeightBase = 100.f;
    MaxWeightModBase = 10.f;
    MaxWeight = MaxWeightBase;
    KnockDownTime = KnockDownBaseTime;
}

float ACharacterCreature::TakeDamage(float DamageAmount, const FDamageEvent &DamageEvent,
                                 class AController *EventInstigator,
                                 AActor *DamageCauser)
{
    if (Cheats.Invulnerable)
        return GetAttributeComponent()->Health->CurrentValue;

    const auto DamageType = DamageEvent.DamageTypeClass.GetDefaultObject();
    const auto DamageBase = Cast<UDamageBase>(DamageType);
    bool isAbsolute = Cast<UAbsoluteDamage>(DamageType) != nullptr;
    float force = 0.f;
    if (DamageEvent.IsOfType(FForceDamageEvent::ClassID) || DamageEvent.IsOfType(FRangedForceDamageEvent::ClassID))
    {
        auto forceType = static_cast<const FForceDamageEvent*>(&DamageEvent);// "I sense a great disturbance in it"
        force = forceType->Force;
    }
    bool blocked = false;
    float balance;
    int32 strMod = UAttributeSetHelper::GetModifier(GetAttributeComponent()->Attributes, EAttributes::Str);
    int32 balanceSkill = USkillHelper::GetValue(GetAttributeComponent()->Skills, ESkill::Balance);

    auto currentStance = CombatComponent->GetCurrentCombatStance();
    // add spread if has ragned weapon equipped
    if (currentStance && !currentStance->IsMelee())
    {
        //if (auto rangedWeapon = Cast<UItemWeaponRanged>(currentStance->GetArmsFromSlot(EArmsSlot::MainHand)))
        if (auto rangedWeapon = Cast<UItemWeaponRanged>(CombatComponent->GetArmItem(EArmsSlot::MainHand)))
            rangedWeapon->AddSpread();
    }

    // check if had shield and what kind
    //auto shield = currentStance ? Cast<UItemWeapon>(currentStance->GetArmsFromSlot(EArmsSlot::Shield)) : nullptr;
    auto shield = currentStance ? Cast<UItemWeapon>(CombatComponent->GetArmItem(EArmsSlot::Shield)) : nullptr;

    bool hasProperShield = shield != nullptr;

    // check actors directions
    bool isFacingAttacker = UCombatComponent::GetAngleBetweenAttackerAndDefender(DamageCauser, this) < CombatComponent->MaxFacingAngle;

    // check parry
    auto attackerChar = Cast<ACharacterCreature>(DamageCauser);
    auto attackerCombatState = attackerChar ? attackerChar->CombatComponent->GetCombatFSM().currentState() : nullptr;
    auto characterCombatStateInfo = *CombatComponent->GetCombatFSM().currentState();

    if (hasProperShield && isFacingAttacker && attackerChar && attackerCombatState && (CombatComponent->GetCombatFSM().isInState(ECombatState::Parry)) &&
        characterCombatStateInfo.CheckAnimInfo(ECombatStateAnimInfo::BlockOrParryActive) &&
        attackerCombatState->CheckAnimInfo(ECombatStateAnimInfo::CanBeParried) &&
        !attackerCombatState->AttackInfo->IsNotParriable)
    {
        attackerChar->InterruptAnimation(ECCState::LossOfBalance, static_cast<int>(ELossOfBalanceCCSubState::WasParried));
        return GetAttributeComponent()->Health->CurrentValue;
    }

    if (CombatComponent->GetCombatFSM().isInState(ECombatState::Block))
    {
        bool attackisInterruptable = attackerChar && attackerCombatState && !attackerCombatState->AttackInfo->IsNotInterruptable;
        bool attackisBlockable = attackerChar && attackerCombatState && !attackerCombatState->AttackInfo->IsNotBlockable;
        if (hasProperShield && isFacingAttacker && attackisBlockable)
        {
            balance = UCombatComponent::CalculateBalance(balanceSkill, strMod, shield->Weight);
            // check if block succeeded
            if (GetAttributeComponent()->Stamina->Consume(CombatComponent->BlockStaminaCost) && (force <= balance))
            {
                if (DamageBase && !isAbsolute)
                {
                    DamageAmount -= shield->DamageLimit.DamageMap[(int32)DamageBase->Type];
                    DamageAmount = DamageAmount * (100 - shield->DamageResistance.DamageMap[(int32)DamageBase->Type]) / 100;
                }
                blocked = true;
                // apply blocked action to the attacker
                if (attackisInterruptable)
                    attackerChar->InterruptAnimation(ECCState::LossOfBalance, static_cast<int>(ELossOfBalanceCCSubState::HitWall));
                // apply blocked action to the defender
                characterCombatStateInfo.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                InterruptAnimation(ECCState::LossOfBalance, static_cast<int>(ELossOfBalanceCCSubState::BlockedAttack));
            }
        }
        else
            balance = UCombatComponent::CalculateBalance(balanceSkill, strMod, 0);
    }
    else
        balance = UCombatComponent::CalculateBalance(balanceSkill, strMod, 0);

    bool isNotInterruptable = CombatComponent->GetCombatFSM().isInState(ECombatState::Attack) &&
         CombatComponent->CombatFSM.currentState()->AttackInfo->IsNotInterruptable;
    if (!blocked && (force > balance) && !isNotInterruptable) // check if block failed and can be interrupted
    {
        if (CombatComponent->KnockdownThreshhold < force / balance) // the consequences
            InterruptAnimation(ECCState::KnockDown);
        else
            InterruptAnimation(ECCState::LossOfBalance, static_cast<int>(ELossOfBalanceCCSubState::None));
    }
    if (!blocked && DamageEvent.IsOfType(FRangedForceDamageEvent::ClassID))  // check if successfully hit with ranged attack in a weak spot
    {
        // TODO finish implement locational hit reactions
        auto damageEv = static_cast<const FRangedForceDamageEvent*>(&DamageEvent);
        switch (damageEv->Type)
        {
        case EWeakSpotType::Head:
            DamageAmount *= 2;  // TODO for now just double the damage
            break;
        default:
            break;
        }
    }

    if (DamageAmount <= 0)
        return GetAttributeComponent()->Health->CurrentValue; 
    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ACharacterCreature::Update(const FCharacterLevelInfo &levelInfo)
{
    Super::Update(levelInfo);
    if (EquipmentComponent->IsUpdateNeeded())
        EquipmentComponent->Update();
    if (CombatComponent->IsUpdateNeeded())
        CombatComponent->Update();
    if (InventoryComponent->IsUpdateNeeded())
        InventoryComponent->Update();
    if (GetMovementComponent()->IsUpdateNeeded())
        GetMovementComponent()->Update();
    if (GetMeshComponent()->IsUpdateNeeded())
        GetMeshComponent()->Update();
    bool strChanged = UAttributeSetHelper::GetAttribute(levelInfo.Attributes, EAttributes::Str) != 0;
    bool acrobaticsChanged = USkillHelper::GetValue(levelInfo.SkillList, ESkill::Tumble) != 0;
    if (strChanged)
        MaxWeight = CalculateMaxWeight();
    if (acrobaticsChanged)
        KnockDownTime = CalculateKnockDownTime();
    // todo other stuff
}
/*
void ACharacterCreature::FireLoadedProjectile()
{
    if (GetCombatComponent()->HasCombatStance())
        if (auto RangedStance = Cast<URangeCombatStance>(GetCombatComponent()->GetCurrentCombatStance()))
            RangedStance->FireLoadedAmmo();
}*/

void ACharacterCreature::StartAnimation(UAnimMontage *AnimMontage,
                                      const FOnMontageBlendingOutStarted &EndCallBack,
                                      float InPlayRate,
                                      FName StartSectionName)
{
    check(AnimMontage != nullptr && "Start Anim: null anim");
    PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
    auto animInstance    = GetMesh()->GetAnimInstance();
    auto montageInstance = animInstance->GetActiveInstanceForMontage(AnimMontage);
    montageInstance->OnMontageBlendingOutStarted = EndCallBack;
}

void ACharacterCreature::InterruptAnimation(ECCState Source, int SubMode)
{
    // already controlled
    if (CCComponent->HasCC())
    {
        //check if highest order bit of source is lower than old
        if (UCCComponent::GetHighestPriorityCC(1 << Source) < CCComponent->CurrentCCPriority())
        {
            return;
        }
    }
    else
    {
        // interrupted by character
        // character can interrupt blocks, spell casts, wall hanging, some attacks and not much else
        if (Source == ECCState::NoCC)
        {
            if (GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
            {
                if (AnimInstance->Montage_IsActive(GetMovementComponent()->HangMontage))
                    AnimInstance->StopSlotAnimation(0.f, FAnimSlotNames::FullBody);
            }
            else if (CombatComponent->GetCombatFSM().isInState(ECombatState::Attack))
            {
                if (AnimInstance->Montage_IsActive(CombatComponent->CombatFSM.currentState()->AttackInfo->AttackMontage))
                    AnimInstance->Montage_Stop(0.f, CombatComponent->CombatFSM.currentState()->AttackInfo->AttackMontage);
            }
            else if (CombatComponent->GetCombatFSM().isInState(ECombatState::Parry))
            {
                if (AnimInstance->Montage_IsActive(AnimInstance->GetMove(ECombatMovementType::Parry)))
                    AnimInstance->Montage_Stop(0.f, AnimInstance->GetMove(ECombatMovementType::Parry));
            }
            if (CombatComponent->GetCombatFSM().isInState(ECombatState::Ability))
            {
                if (AnimInstance->Montage_IsActive(AbilityManager->CurrentAbility->CastAnimMontage))
                    AnimInstance->Montage_Stop(0.f, AbilityManager->CurrentAbility->CastAnimMontage);
            }
            return;
        }
        // interrupted by CC
        if (GetMovementComponent()->IsTransitionInProgress())
            GetMovementComponent()->TransitionInterrupt();
        // something else
    }
    if (CCComponent->IsInCCMode(Source))
    {// if already in this mode then remove end callback before restart
        FAnimMontageInstance* montageInstance = nullptr;
        //if (Source == ECCState::LossOfBalance && SubMode != ELossOfBalanceCCSubState::None)
        if (Source == ECCState::LossOfBalance && static_cast<ELossOfBalanceCCSubState>(SubMode) != ELossOfBalanceCCSubState::None)
        {
            if (CCComponent->IsInCCSubMode(SubMode))
                montageInstance = AnimInstance->GetActiveInstanceForMontage((AnimInstance->CCLOBAnims[SubMode]));
                //montageInstance = AnimInstance->GetActiveInstanceForMontage((CCComponent->CCLOBAnims[static_cast<ELossOfBalanceCCSubState>(SubMode)]));
                //montageInstance = AnimInstance->GetActiveInstanceForMontage((CCLOBAnims[static_cast<ELossOfBalanceCCSubState>(SubMode)]));
        }
        else
            montageInstance = AnimInstance->GetActiveInstanceForMontage((CCComponent->CCAnims[Source]));
        if (montageInstance)
            montageInstance->OnMontageBlendingOutStarted.Unbind();
    }
    // TODO devise other times
    if (Source == ECCState::KnockDown)
        CCComponent->SetCCMode(Source, SubMode, &KnockDownTime);
    else
        CCComponent->SetCCMode(Source, SubMode);
    FOnMontageBlendingOutStarted callBack;

    callBack.BindUObject(this,CCInterruptDelegates[static_cast<ECCState>(CCComponent->CurrentCCPriority())]);  
    //if (Source == ECCState::LossOfBalance && SubMode != ELossOfBalanceCCSubState::None)
        //StartAnimation(AnimInstance->CCLOBAnims[static_cast<ELossOfBalanceCCSubState>(SubMode)], callBack, 1.f);
    if (Source == ECCState::LossOfBalance && static_cast<ELossOfBalanceCCSubState>(SubMode) != ELossOfBalanceCCSubState::None)
        StartAnimation(AnimInstance->CCLOBAnims[SubMode], callBack, 1.f);
        //StartAnimation(CCComponent->CCLOBAnims[static_cast<ELossOfBalanceCCSubState>(SubMode)], callBack, 1.f);
    else
        StartAnimation(CCComponent->CCAnims[Source], callBack, 1.f); 
}

UHumanoidMovementComponent *ACharacterCreature::GetMovementComponent() const
{
    return Cast<UHumanoidMovementComponent>(ACharacter::GetMovementComponent());
}

UCharacterMeshComponent *ACharacterCreature::GetMeshComponent() const
{
    return Cast<UCharacterMeshComponent>(ACharacter::GetMesh());
}

void ACharacterCreature::DoLevelUp(const FCharacterLevelInfo &levelInfo)
{
    GetAttributeComponent()->LevelUp(levelInfo);
    bool dexChanged = UAttributeSetHelper::GetAttribute(levelInfo.Attributes, EAttributes::Dex) != 0;
    bool strChanged = UAttributeSetHelper::GetAttribute(levelInfo.Attributes, EAttributes::Str) != 0;
    bool acrobaticsChanged = USkillHelper::GetValue(levelInfo.SkillList, ESkill::Tumble) != 0;
    if (dexChanged || strChanged || acrobaticsChanged)
        GetMovementComponent()->SetUpdate(true);
    Update(levelInfo);
    // todo other stuff (combat, ability manager)
}

bool ACharacterCreature::PickupItem(AItemActor *ItemActor)
{
    if(!IsValid(ItemActor)) return false;
    if(!IsValid(ItemActor->ItemBase)) return false;

    auto result = InventoryComponent->AddToInventory(ItemActor->ItemBase,ItemActor->StackCount);

    if(result)
    {
        ItemActor->Destroy();
    }

    return result;
}

float ACharacterCreature::CalculateKnockDownTime() const
{
    int skillVal = USkillHelper::GetValue(GetAttributeComponent()->Skills, ESkill::Tumble);
    float weightMod = EquipmentComponent->CurrentWeight / MaxWeight;
    return KnockDownBaseTime - KnockDownModBase * skillVal - KnockDownWeightModBase * weightMod;
}

void ACharacterCreature::UpdateCCLOBAnims()
{
    /*
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::HitWall, AnimInstance->CurrentHitWallAnim);
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::WasParried, AnimInstance->CurrentWasParriedAnim);
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::BlockedAttack, AnimInstance->CurrentBlockedAttackAnim);
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::Chamber, AnimInstance->CurrentChamberAnim);
       
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::HitWall, AnimInstance->GetMove(ECombatMovementType::HitWall));
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::WasParried, AnimInstance->GetMove(ECombatMovementType::WasParried));
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::BlockedAttack, AnimInstance->GetMove(ECombatMovementType::BlockedAttack));
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::Chamber, AnimInstance->GetMove(ECombatMovementType::Chamber));    

    CCComponent->CCLOBAnims.Emplace(ELossOfBalanceCCSubState::HitWall, AnimInstance->CurrentHitWallAnim);
    CCComponent->CCLOBAnims.Emplace(ELossOfBalanceCCSubState::WasParried, AnimInstance->CurrentWasParriedAnim);
    CCComponent->CCLOBAnims.Emplace(ELossOfBalanceCCSubState::BlockedAttack, AnimInstance->CurrentBlockedAttackAnim);
    CCComponent->CCLOBAnims.Emplace(ELossOfBalanceCCSubState::Chamber, AnimInstance->CurrentChamberAnim);
    */
}

void ACharacterCreature::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    AnimInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
    check(AnimInstance && "Invalid anim blueprint");
    CCComponent->CCAnims.Emplace(ECCState::KnockDown, AnimInstance->KnockDownAnim);
    CCComponent->CCAnims.Emplace(ECCState::LossOfBalance, AnimInstance->LossOfBalanceAnim);
    CCComponent->CCAnims.Emplace(ECCState::Stun, AnimInstance->StunAnim);// TODO add sleep and stasis
    UpdateCCLOBAnims();
}

void ACharacterCreature::BeginPlay()
{
    ACharacter::BeginPlay();
    //CCStateManager.Init(this);
    //Logger.Init(this);
    // init components
    GetAttributeComponent()->SetUpdate();
    GetMovementComponent()->SetUpdate(true);
    Update(FCharacterLevelInfo());
}


void ACharacterCreature::CCStunEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    // todo reset some armor debuffs
    CCComponent->ResetCCMode(ECCState::Stun);
}

void ACharacterCreature::CCLOBEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    if (CCComponent->IsInCCSubMode(static_cast<int>(ELossOfBalanceCCSubState::BlockedAttack)))
    {
        if (CombatComponent->IsBlockButtonPressed())
        {
            FOnMontageBlendingOutStarted callBack;
            callBack.BindUObject(CombatComponent, &UCombatComponent::OnBlockEnded);
            StartAnimation(AnimInstance->GetMove(ECombatMovementType::Block), callBack, CombatComponent->AttackSpeed, FAnimSectionNames::Loop);
        }
        else
        {
            CombatComponent->GetCombatFSM().currentState()->ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
            CombatComponent->UpdateCombatFSM();
        }
    }
    else if (CCComponent->IsInCCSubMode(static_cast<int>(ELossOfBalanceCCSubState::Chamber)))
    {
        //auto& stateInfo = CombatComponent->GetCombatFSM().currentState()->Info;
        auto& stateInfo = *CombatComponent->GetCombatFSM().currentState();
        bool startedAttack = !stateInfo.CheckAnimInfo(ECombatStateAnimInfo::CheckedForChamber) && stateInfo.CheckAnimInfo(ECombatStateAnimInfo::CanCounterChamber);
        if (!startedAttack)
        { // check if starting new attack from chamber then don't interrupt
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
            CombatComponent->UpdateCombatFSM();
        }
        else
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::CanCounterChamber, false);
    }
    CCComponent->ResetCCMode(ECCState::LossOfBalance);
}

void ACharacterCreature::CCKnockDownEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    // todo reset some armor debuffs
//<<<<<<< HEAD
    //CCStateManager.ResetCCMode(ECCState::KnockDown);
}

