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

    Logger.RegisterSubLogger(GetMovementComponent(), true);
    Logger.RegisterSubLogger(CombatComponent, true);
    Logger.RegisterSubLogger(EquipmentComponent, true);
    Logger.RegisterSubLogger(InventoryComponent, true);
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
    if (DamageEvent.IsOfType(FForceDamageEvent::ClassID))
    { 
        auto forceType = static_cast<const FForceDamageEvent*>(&DamageEvent);// "I sense a great disturbance in it" TODO because there is a potential bug if it's not FForceDamageEvent
        force = forceType->Force;
    }
    bool blocked = false;
    float balance;
    int32 strMod = UAttributeSetHelper::GetModifier(GetAttributeComponent()->Attributes, EAttributes::Str);
    int32 balanceSkill = USkillHelper::GetValue(GetAttributeComponent()->Skills, ESkill::Balance);
    Logger.Log(this, *FString::Printf(TEXT("take damage before block %f"), DamageAmount));
    auto currentStance = CombatComponent->GetCurrentCombatStance();
    // add spread if has ragned weapon equipped
    if (currentStance && !currentStance->IsMelee())
    {
        if (auto rangedWeapon = Cast<UItemWeaponRanged>(currentStance->GetArmsFromSlot(EArmsSlot::MainHand)))
            rangedWeapon->AddSpread();
    }
    // check if had shield and what kind
    auto shield = currentStance ? Cast<UItemWeapon>(currentStance->GetArmsFromSlot(EArmsSlot::Shield)) : nullptr;
    bool hasProperShield = shield != nullptr;
    // check actors directions
    bool isFacingAttacker = UCombatComponent::GetAngleBetweenAttackerAndDefender(DamageCauser, this) < CombatComponent->MaxFacingAngle;
    // check parry
    auto attackerChar = Cast<ACharacterCreature>(DamageCauser);
    auto attackerCombatState = attackerChar ? attackerChar->CombatComponent->GetCombatFSM().currentState() : nullptr;
    auto characterCombatStateInfo = CombatComponent->GetCombatFSM().currentState()->Info;
    if (hasProperShield && isFacingAttacker && attackerChar && attackerCombatState && (CombatComponent->GetCombatFSM().isInState(ECombatState::Parry)) &&
        characterCombatStateInfo.CheckAnimInfo(ECombatStateAnimInfo::BlockOrParryActive) &&
        attackerCombatState->Info.CheckAnimInfo(ECombatStateAnimInfo::CanBeParried) &&
        !attackerCombatState->Info.AttackInfo->IsNotParriable)
    {
        attackerChar->InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::WasParried);
        return GetAttributeComponent()->Health->CurrentValue;
    }
    if (CombatComponent->GetCombatFSM().isInState(ECombatState::Block))
    {
        bool attackisInterruptable = attackerChar && attackerCombatState && !attackerCombatState->Info.AttackInfo->IsNotInterruptable;
        bool attackisBlockable = attackerChar && attackerCombatState && !attackerCombatState->Info.AttackInfo->IsNotBlockable;
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
                    attackerChar->InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::HitWall);
                // apply blocked action to the defender
                characterCombatStateInfo.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::BlockedAttack);
            }
        }
        else
            balance = UCombatComponent::CalculateBalance(balanceSkill, strMod, 0);
    }
    else
        balance = UCombatComponent::CalculateBalance(balanceSkill, strMod, 0);
    Logger.Log(this,*FString::Printf(TEXT("force %f"), force));
    Logger.Log(this,*FString::Printf(TEXT("balance %f"), balance));
    bool isNotInterruptable = CombatComponent->GetCombatFSM().isInState(ECombatState::Attack) &&
        CombatComponent->GetCombatFSM().currentState()->Info.AttackInfo->IsNotInterruptable;
    if (!blocked && (force > balance) && !isNotInterruptable) // check if block failed and can be interrupted
    {
        if (CombatComponent->KnockdownThreshhold < force / balance) // the consequences
            InterruptAnimation(ECCState::KnockDown);
        else
            InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::None);
    }
    Logger.Log(this,*FString::Printf(TEXT("take damage after block %f"), DamageAmount));
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
    if (CCStateManager.HasCC())
    {
        //check if highest order bit of source is lower than old
        if (FCCStateManager::GetHighestPriorityCC(1 << Source) < CCStateManager.CurrentCCPriority())
        {

            Logger.Log(this, *FString::Printf(TEXT("InterruptAnimation: cant interrupt CC, low priority")));
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
                if (AnimInstance->Montage_IsActive(CombatComponent->GetCombatFSM().currentState()->Info.AttackInfo->AttackMontage))
                    AnimInstance->Montage_Stop(0.f, CombatComponent->GetCombatFSM().currentState()->Info.AttackInfo->AttackMontage);
            }
            else if (CombatComponent->GetCombatFSM().isInState(ECombatState::Parry))
            {
                if (AnimInstance->Montage_IsActive(AnimInstance->CurrentParryAnim))
                    AnimInstance->Montage_Stop(0.f, AnimInstance->CurrentParryAnim);
            }
            if (CombatComponent->GetCombatFSM().isInState(ECombatState::Ability))
            {
                if (AnimInstance->Montage_IsActive(
                      CombatComponent->CurrentAbility->CastAnimMontage))
                    AnimInstance->Montage_Stop(
                      0.f, CombatComponent->CurrentAbility->CastAnimMontage);
            }
            return;
        }
        // interrupted by CC
        if (GetMovementComponent()->IsTransitionInProgress())
            GetMovementComponent()->TransitionInterrupt();
        // something else
    }
    if (CCStateManager.IsInCCMode(Source))
    {// if already in this mode then remove end callback before restart
        FAnimMontageInstance* montageInstance = nullptr;
        if (Source == ECCState::LossOfBalance && SubMode != ELossOfBalanceCCSubState::None)
        {
            if (CCStateManager.IsInCCSubMode(SubMode))
                montageInstance = AnimInstance->GetActiveInstanceForMontage((CCLOBAnims[static_cast<ELossOfBalanceCCSubState>(SubMode)]));
        }
        else
            montageInstance = AnimInstance->GetActiveInstanceForMontage((CCAnims[Source]));
        if (montageInstance)
            montageInstance->OnMontageBlendingOutStarted.Unbind();
    }
    // TODO devise other times
    if (Source == ECCState::KnockDown)
        CCStateManager.SetCCMode(Source, SubMode, &KnockDownTime);
    else
        CCStateManager.SetCCMode(Source, SubMode);
    FOnMontageBlendingOutStarted callBack;
    callBack.BindUObject(this,
        CCInterruptDelegates[static_cast<ECCState>(CCStateManager.CurrentCCPriority())]);  
    if (Source == ECCState::LossOfBalance && SubMode != ELossOfBalanceCCSubState::None)
        StartAnimation(CCLOBAnims[static_cast<ELossOfBalanceCCSubState>(SubMode)], callBack, 1.f);
    else
        StartAnimation(CCAnims[Source], callBack, 1.f); 
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
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::HitWall, AnimInstance->CurrentHitWallAnim);
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::WasParried, AnimInstance->CurrentWasParriedAnim);
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::BlockedAttack, AnimInstance->CurrentBlockedAttackAnim);
    CCLOBAnims.Emplace(ELossOfBalanceCCSubState::Chamber, AnimInstance->CurrentChamberAnim);
}

void ACharacterCreature::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    AnimInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
    check(AnimInstance && "Invalid anim blueprint");
    CCAnims.Emplace(ECCState::KnockDown, AnimInstance->KnockDownAnim);
    CCAnims.Emplace(ECCState::LossOfBalance, AnimInstance->LossOfBalanceAnim);
    CCAnims.Emplace(ECCState::Stun, AnimInstance->StunAnim);// TODO add sleep and stasis
    UpdateCCLOBAnims();
}

void ACharacterCreature::BeginPlay()
{
    ACharacter::BeginPlay();

    CCStateManager.Init(this);
    Logger.Init(this);
    // init components
    GetAttributeComponent()->SetUpdate();
    GetMovementComponent()->SetUpdate(true);
    Update(FCharacterLevelInfo());
}


void ACharacterCreature::CCStunEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    // todo reset some armor debuffs
    Logger.Log(this,*FString::Printf(TEXT("CCEnd: Stun")));
    CCStateManager.ResetCCMode(ECCState::Stun);
}

void ACharacterCreature::CCLOBEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    Logger.Log(this,*FString::Printf(TEXT("CCEnd: Loss of balance")));
    if (CCStateManager.IsInCCSubMode(BlockedAttack))
    {
        if (CombatComponent->IsBlockButtonPressed())
        {
            FOnMontageBlendingOutStarted callBack;
            callBack.BindUObject(CombatComponent, &UCombatComponent::OnBlockEnded);
            StartAnimation(AnimInstance->CurrentBlockAnim, callBack, CombatComponent->AttackSpeed, FAnimSectionNames::Loop);
        }
        else
        {
            CombatComponent->GetCombatFSM().currentState()->Info.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
            CombatComponent->UpdateCombatFSM();
        }
    }
    else if (CCStateManager.IsInCCSubMode(Chamber))
    {
        auto& stateInfo = CombatComponent->GetCombatFSM().currentState()->Info;
        bool startedAttack = !stateInfo.CheckAnimInfo(ECombatStateAnimInfo::CheckedForChamber) && stateInfo.CheckAnimInfo(ECombatStateAnimInfo::CanCounterChamber);
        if (!startedAttack)
        { // check if starting new attack from chamber then don't interrupt
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
            CombatComponent->UpdateCombatFSM();
        }
        else
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::CanCounterChamber, false);
    }
    CCStateManager.ResetCCMode(ECCState::LossOfBalance);
}

void ACharacterCreature::CCKnockDownEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    // todo reset some armor debuffs
    Logger.Log(this,*FString::Printf(TEXT("CCEnd: KnockDown")));
    CCStateManager.ResetCCMode(ECCState::KnockDown);
}

int FCCStateManager::GetHighestPriorityCC(int bitmask)
{
#if PLATFORM_WINDOWS
    unsigned long index;
    _BitScanReverse(&index, bitmask);
    return index;
#else
    return ffs(bitmask)-1;
#endif
}

FCCStateManager::FCCStateManager()
{
    CCLockTimeDivider = 2.f;
    CCLockTime = 10.f;
    CurrentCCState = ECCState::NoCC;
}

void FCCStateManager::SetCCMode(ECCState Mode, unsigned int SubMode, float *Time)
{
    Character->GetWorld()->GetTimerManager().ClearTimer(CCLocks[Mode].Key.StunLockTimer);
    FTimerDelegate lockDelegate = FTimerDelegate::CreateLambda([this, Mode]() { ResetCCLock(Mode); });
    Character->GetWorld()->GetTimerManager().SetTimer(CCLocks[Mode].Key.StunLockTimer, lockDelegate, CCLockTime, false );
    if (Time != nullptr)
    {
        float duration = *Time * GetCCLockTimeModifier(Mode);
        Character->Logger.Log(Character,*FString::Printf(TEXT("Setting CC with duration of %f"), duration));
        FTimerDelegate CCDelegate = FTimerDelegate::CreateLambda([this, Mode]() 
        {
            if (Character->AnimInstance->Montage_IsActive(Character->CCAnims[Mode]))
                Character->AnimInstance->Montage_Stop(0.f, Character->CCAnims[Mode]);
        });
        Character->GetWorld()->GetTimerManager().SetTimer(CCLocks[Mode].Key.CCTimer, CCDelegate, duration, false);
    }
    CCLocks[Mode].Value++;
    CurrentCCState |= 1u << Mode;
    CurrentCCSubState |= 1u << SubMode;
    Character->OnCCStateChanged.Broadcast(Mode, SubMode);
}

void FCCStateManager::ResetCCMode(ECCState Mode)
{
    if (IsInCCMode(Mode))
    {
        CurrentCCState &= ~(1u << Mode);
        CurrentCCSubState = 0u;
        FTimerDelegate lockDelegate = FTimerDelegate::CreateLambda([this, Mode]() { ResetCCLock(Mode); });
        Character->GetWorld()->GetTimerManager().SetTimer(CCLocks[Mode].Key.StunLockTimer, lockDelegate, CCLockTime, false );
        Character->OnCCStateChanged.Broadcast(0u, 0u);
    }
}

float FCCStateManager::GetCCLockTimeModifier(ECCState Mode) const
{
    return CCLocks[Mode].Value ? 1.f / (CCLocks[Mode].Value * CCLockTimeDivider) : 1.f;
}

void FCCStateManager::ResetCCLock(int index)
{
    CCLocks[index].Value = 0;
}
