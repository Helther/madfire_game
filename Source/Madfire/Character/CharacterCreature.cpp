#include "CharacterCreature.h"
#include "CharacterAnimInstance.h"


ACharacterCreature::ACharacterCreature(const FObjectInitializer &ObjectInitializer) :
    Super(ObjectInitializer.SetDefaultSubobjectClass<UHumanoidMovementComponent>(
      ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;
    CombatComponent = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
    EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>("EquipmentComponent");
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");
    CCInterruptDelegates.Emplace(ECCState::LossOfBalance, &ACharacterCreature::CCLOBEnded);
    CCInterruptDelegates.Emplace(ECCState::Stun, &ACharacterCreature::CCStunEnded);
    CCInterruptDelegates.Emplace(ECCState::KnockDown, &ACharacterCreature::CCKnockDownEnded);
    // TODO add other callbacks
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

float ACharacterCreature::TakeDamage(float DamageAmount, const struct FDamageEvent &DamageEvent,
                                 class AController *EventInstigator,
                                 AActor *DamageCauser)
{
    if (Cheats.Invulnerable)
        return AttributeComponent->Health->CurrentValue;

    const auto DamageType = DamageEvent.DamageTypeClass.GetDefaultObject();
    const auto Type = Cast<UDamageBase>(DamageType)->Type;
    bool isAbsolute = Cast<UAbsoluteDamage>(DamageType) != nullptr;
    float force = static_cast<const FForceDamageEvent*>(&DamageEvent)->Force;// "I sense a great disturbance in it"
    bool blocked = false;
    float balance;
    int32 strMod = UAttributeSetHelper::GetModifier(AttributeComponent->Attributes, EAttributes::Str);
    int32 balanceSkill = USkillHelper::GetValue(AttributeComponent->Skills, ESkill::Balance);
    Logger.Log(this,*FString::Printf(TEXT("take damage before block %f"), DamageAmount));
    auto attackerChar = Cast<ACharacterCreature>(DamageCauser);
    // check if had shield and what kind
    UItemWeapon* shield = nullptr;
    if (CombatComponent->CurrentWeaponOffH != nullptr && CombatComponent->CurrentWeaponOffH->WeaponType == EWeaponType::Shield)
        shield = CombatComponent->CurrentWeaponOffH;
    else if (CombatComponent->CurrentWeaponMainH != nullptr && CombatComponent->CurrentWeaponMainH->WeaponType != EWeaponType::Unarmed)
        shield = CombatComponent->CurrentWeaponMainH;
    bool hasShield = shield != nullptr;
    // check actors directions
    bool isFacingAttacker = UCombatComponent::GetAngleBetweenAttackerAndDefender(DamageCauser, this) < CombatComponent->MaxFacingAngle;
    // check parry
    if (hasShield && isFacingAttacker && attackerChar && (CombatComponent->CombatFSM.currentState()->Id == ECombatState::Parry) &&
        CombatComponent->CombatFSM.currentState()->Info.CheckAnimInfo(ECombatStateAnimInfo::BlockOrParryActive) &&
        attackerChar->CombatComponent->CombatFSM.currentState()->Info.CheckAnimInfo(ECombatStateAnimInfo::CanBeParried))
    {
        attackerChar->InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::WasParried);
        return AttributeComponent->Health->CurrentValue; 
    }
    if (CombatComponent->CombatFSM.currentState()->Id == ECombatState::Block)
    {
        if (hasShield && isFacingAttacker)
        {
            balance = UCombatComponent::CalculateBalance(balanceSkill, strMod, shield->Weight);
            // check if block succeeded
            if (AttributeComponent->Stamina->Consume(CombatComponent->BlockStaminaCost) && (force <= balance))
            {
                if (!isAbsolute)
                {
                    DamageAmount -= shield->DamageLimit.DamageMap[(int32)Type];
                    DamageAmount = DamageAmount * (100 - shield->DamageResistance.DamageMap[(int32)Type]) / 100;
                }
                blocked = true;
                // apply blocked action to the attacker
                if (attackerChar)
                    attackerChar->InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::HitWall);
                // apply blocked action to the defender
                CombatComponent->CombatFSM.currentState()->Info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::BlockedAttack);
            }
            else
                force *= CombatComponent->HitForceReductionMod;// force reduction from block attempt
        }
        else
            balance = UCombatComponent::CalculateBalance(balanceSkill, strMod, 0);
    }
    else
        balance = UCombatComponent::CalculateBalance(balanceSkill, strMod, 0);
    Logger.Log(this,*FString::Printf(TEXT("force %f"), force));
    Logger.Log(this,*FString::Printf(TEXT("balance %f"), balance));
    if (!blocked && (force > balance)) // check if block failed
    {
        if (CombatComponent->KnockdownThreshhold < force / balance) // the consequences
            InterruptAnimation(ECCState::KnockDown);
        else
            InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::None);
    }
    Logger.Log(this,*FString::Printf(TEXT("take damage after block %f"), DamageAmount));
    if (DamageAmount <= 0)
        return AttributeComponent->Health->CurrentValue; 
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
            auto animInstance = GetMesh()->GetAnimInstance();
            if (GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
            {
                if (animInstance->Montage_IsActive(GetMovementComponent()->HangMontage))
                    animInstance->StopSlotAnimation(0.f, FAnimSlotNames::FullBody);
            }
            else if (CombatComponent->CombatFSM.currentState()->Id
                     == ECombatState::Attack)
            {
                if (animInstance->Montage_IsActive(
                      CombatComponent->CombatFSM.currentState()
                        ->Info.AttackInfo->AttackMontage))
                    animInstance->Montage_Stop(0.f,
                                               CombatComponent->CombatFSM.currentState()
                                                 ->Info.AttackInfo->AttackMontage);
            }
            else if (CombatComponent->CombatFSM.currentState()->Id == ECombatState::Parry)
            {
                if (animInstance->Montage_IsActive(
                      CombatComponent->GetCurrentCombatStance()->CurrentParryAnim))
                    animInstance->Montage_Stop(
                      0.f, CombatComponent->GetCurrentCombatStance()->CurrentParryAnim);
            }
            if (CombatComponent->CombatFSM.currentState()->Id == ECombatState::Ability)
            {
                if (animInstance->Montage_IsActive(
                      CombatComponent->CurrentAbility->CastAnimMontage))
                    animInstance->Montage_Stop(
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
        auto animInstance    = GetMesh()->GetAnimInstance();
        auto montageInstance = animInstance->GetActiveInstanceForMontage((CCAnims[Source]));
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

void ACharacterCreature::DoLevelUp(const FCharacterLevelInfo &levelInfo)
{
    AttributeComponent->LevelUp(levelInfo);
    bool dexChanged = UAttributeSetHelper::GetAttribute(levelInfo.Attributes, EAttributes::Dex) != 0;
    bool strChanged = UAttributeSetHelper::GetAttribute(levelInfo.Attributes, EAttributes::Str) != 0;
    bool acrobaticsChanged = USkillHelper::GetValue(levelInfo.SkillList, ESkill::Tumble) != 0;
    if (dexChanged || strChanged || acrobaticsChanged)
        GetMovementComponent()->SetDirty();
    Update(levelInfo);
    // todo other stuff (combat, ability manager)
}

float ACharacterCreature::CalculateKnockDownTime() const
{
    int skillVal = USkillHelper::GetValue(AttributeComponent->Skills, ESkill::Tumble);
    float weightMod = EquipmentComponent->CurrentWeight / MaxWeight;
    return KnockDownBaseTime - KnockDownModBase * skillVal - KnockDownWeightModBase * weightMod;
}

void ACharacterCreature::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    auto animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
    if (animInstance)
    {
        CCAnims.Emplace(ECCState::KnockDown, animInstance->KnockDownAnim);
        CCAnims.Emplace(ECCState::LossOfBalance, animInstance->LossOfBalanceAnim);
        CCAnims.Emplace(ECCState::Stun, animInstance->StunAnim);
        CCLOBAnims.Emplace(ELossOfBalanceCCSubState::HitWall, animInstance->HitWallAnim);
        CCLOBAnims.Emplace(ELossOfBalanceCCSubState::WasParried, animInstance->WasParriedAnim);
        CCLOBAnims.Emplace(ELossOfBalanceCCSubState::BlockedAttack, animInstance->BlockedAttackAnim);
        CCLOBAnims.Emplace(ELossOfBalanceCCSubState::Chamber, animInstance->ChamberAnim);
    }
}

void ACharacterCreature::BeginPlay()
{
    ACharacter::BeginPlay();

    CCStateManager.Init(this);
    Logger.Init(this);
    // init components
    AttributeComponent->SetDirty();
    GetMovementComponent()->SetDirty();
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
    // todo 
    Logger.Log(this,*FString::Printf(TEXT("CCEnd: Loss of balance")));
    if (CCStateManager.IsInCCSubMode(BlockedAttack))
    {
        if (CombatComponent->IsBlockButtonPressed())
        {
            FOnMontageBlendingOutStarted callBack;
            callBack.BindUObject(CombatComponent, &UCombatComponent::OnBlockEnded);
            StartAnimation(CombatComponent->GetCurrentCombatStance()->CurrentBlockAnim, callBack, CombatComponent->AttackSpeed, FAnimSectionNames::Loop);
        }
        else
        {
            CombatComponent->CombatFSM.currentState()->Info.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
            CombatComponent->UpdateCombatFSM();
        }
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
    return ffs(bitmask);
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
            auto animInstance = Character->GetMesh()->GetAnimInstance();
            if (animInstance->Montage_IsActive(Character->CCAnims[Mode]))
                animInstance->Montage_Stop(0.f, Character->CCAnims[Mode]);
        });
        Character->GetWorld()->GetTimerManager().SetTimer(CCLocks[Mode].Key.CCTimer, CCDelegate, duration, false);
    }
    CCLocks[Mode].Value++;
    CurrentCCState |= 1u << Mode;
    CurrentCCSubState |= 1u << SubMode;
}

void FCCStateManager::ResetCCMode(ECCState Mode)
{
    if (IsInCCMode(Mode))
    {
        CurrentCCState &= ~(1u << Mode);
        CurrentCCSubState = 0u;
        FTimerDelegate lockDelegate = FTimerDelegate::CreateLambda([this, Mode]() { ResetCCLock(Mode); });
        Character->GetWorld()->GetTimerManager().SetTimer(CCLocks[Mode].Key.StunLockTimer, lockDelegate, CCLockTime, false );
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
