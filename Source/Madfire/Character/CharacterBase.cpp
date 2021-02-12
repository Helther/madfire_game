#include "CharacterBase.h"

#include "CharacterAnimInstance.h"


ACharacterBase::ACharacterBase(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseMovementComponent>(
    ACharacter::CharacterMovementComponentName))
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    AttributesComponent =
      CreateDefaultSubobject<UAttributesComponent>("AttributesComponent");
    LevelComponent  = CreateDefaultSubobject<ULevelComponent>("LevelComponent");
    CombatComponent = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
    EquipmentComponent =
      CreateDefaultSubobject<UEquipmentComponent>("EquipmentComponent");
    InventoryComponent =
      CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");
    CurrentCCState = ECCState::NoCC;
    MaxWeightMod   = 10.f;
    // todo add more
    CCInterruptDelegates.Emplace(ECCState::Sleep, &ACharacterBase::CCStunEnded);
    CCInterruptDelegates.Emplace(ECCState::LossOfBalance, &ACharacterBase::CCStunEnded);
    CCInterruptDelegates.Emplace(ECCState::Stun, &ACharacterBase::CCStunEnded);
    CCInterruptDelegates.Emplace(ECCState::KnockDown,
                                 &ACharacterBase::CCKnockDownEnded);
    CCInterruptDelegates.Emplace(ECCState::Stasis, &ACharacterBase::CCStatisEnded);
}

UBaseMovementComponent *ACharacterBase::GetMovementComponent() const
{
    return Cast<UBaseMovementComponent>(ACharacter::GetMovementComponent());
}

UCombatComponent *ACharacterBase::GetCombatComponent() const
{
    check(CombatComponent != nullptr && "No CombatComponent");
    return CombatComponent;
}

UAttributesComponent *ACharacterBase::GetAttributesComponent() const
{
    check(AttributesComponent != nullptr && "No AttrComponent");
    return AttributesComponent;
}

ULevelComponent *ACharacterBase::GetLevelComponent() const
{
    check(LevelComponent != nullptr && "No LevelComponent");
    return LevelComponent;
}

UEquipmentComponent *ACharacterBase::GetEquipmentComponent() const
{
    check(EquipmentComponent != nullptr && "No EquipmentComponent");
    return EquipmentComponent;
}

UInventoryComponent *ACharacterBase::GetInventoryComponent() const
{
    check(InventoryComponent != nullptr && "No InventoryComponent");
    return InventoryComponent;
}

float ACharacterBase::TakeDamage(float DamageAmount,
                                 const struct FDamageEvent &DamageEvent,
                                 class AController *EventInstigator,
                                 AActor *DamageCauser)
{
    const auto DamageType = DamageEvent.DamageTypeClass.GetDefaultObject();

    const auto PhysD  = Cast<UPhysicDamage>(DamageType);
    const auto MagicD = Cast<UMagicDamage>(DamageType);

    if (PhysD != nullptr)
    {
        auto Type  = PhysD->Type;
        auto Force = PhysD->Force;
        DamageAmount -=
          GetAttributesComponent()->CurrentDamageAbsorb.DamageMap[(int32)Type];
        DamageAmount = DamageAmount
          * (GetAttributesComponent()->CurrentDamageResistance.DamageMap[(int32)Type]
             - 100)
          / 100;
        GetAttributesComponent()->Health->Consume(DamageAmount); // -> bool
        //TODO: Death, Force, DamageItem
        return DamageAmount;
    }
    else if (MagicD != nullptr)
    { // тоже что и физический + проверка на пробитие магического щита если такой имеется
        auto Type        = MagicD->Type;
        auto MagicSchool = MagicD->MagicSchool;
        DamageAmount -=
          GetAttributesComponent()->CurrentDamageAbsorb.DamageMap[(int32)Type];
        DamageAmount = DamageAmount
          * (GetAttributesComponent()->CurrentDamageResistance.DamageMap[(int32)Type]
             - 100)
          / 100;
        if (DamageAmount < 0)
        {
            if (GetAttributesComponent()->Health->Consume(DamageAmount))
                return DamageAmount = 0;
            else
                return DamageAmount - GetAttributesComponent()->MagicShield->CurrentVal;
        }
        return DamageAmount = 0;
    }
    else if (DamageType) //if damage nor Physical or Magical, then itВґs raw Damage
    {
        GetAttributesComponent()->Health->Consume(-DamageAmount); // -> bool
        return DamageAmount;
    }

    return 0;
}
void ACharacterBase::OnJumped_Implementation()
{
    GetMovementComponent()->bCanHangAgain   = false;
    GetMovementComponent()->bWasJustJumping = true;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay() { Super::BeginPlay(); }

void ACharacterBase::OnUpdate() { this->Update(); }

void ACharacterBase::Update()
{
    if (GetLevelComponent()->IsUpdateNeeded())
        GetLevelComponent()->Update();
    if (GetAttributesComponent()->IsUpdateNeeded())
    {
        GetAttributesComponent()->Update();
        CalculateMaxWeight();
    }
    if (GetEquipmentComponent()->IsUpdateNeeded())
        GetEquipmentComponent()->Update();
    if (GetInventoryComponent()->IsUpdateNeeded())
        GetInventoryComponent()->Update();
    if (GetMovementComponent()->IsUpdateNeeded())
        GetMovementComponent()->Update();

    //other calculation
}

void ACharacterBase::DoLevelUp(const FLevelInfo &levelInfo)
{
    GetLevelComponent()->LevelUp(levelInfo);
    GetLevelComponent()->SetUpdate();
    GetAttributesComponent()->SetUpdate();
    bool dexChanged =
      UAttributesHelper::GetAttribute(levelInfo.Attributes, EAttributes::Dex) != 0;
    bool strChanged =
      UAttributesHelper::GetAttribute(levelInfo.Attributes, EAttributes::Str) != 0;
    bool acrobaticsChanged =
      USkillHelper::GetValue(levelInfo.SkillList, ESkill::Tumble) != 0;
    if (strChanged)
    {
        GetInventoryComponent()->SetUpdate();
        GetEquipmentComponent()->SetUpdate();
    }
    if (dexChanged || strChanged || acrobaticsChanged)
        GetMovementComponent()->SetUpdate();
    // может что то еще
    Update();
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void ACharacterBase::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    auto animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
    CCAnims.Emplace(ECCState::KnockDown, animInstance->KnockDownAnim);
    CCAnims.Emplace(ECCState::LossOfBalance, animInstance->LossOfBalanceAnim);
    CCAnims.Emplace(ECCState::Sleep, animInstance->SleepAnim);
    CCAnims.Emplace(ECCState::Stasis, animInstance->StasisAnim);
    CCAnims.Emplace(ECCState::Stun, animInstance->StunAnim);
}


void ACharacterBase::CalculateMaxWeight()
{
    const int32 str = UAttributesHelper::GetAttribute(
      GetAttributesComponent()->CurrentAttributes, EAttributes::Str);
    MaxWeight = MaxWeightMod * str; // clever formula!
}

void ACharacterBase::StartAnimTest(float InPlayRate)
{
    FOnMontageBlendingOutStarted callBack;
    callBack.BindUObject(GetMovementComponent(), &UBaseMovementComponent::OnParkourEnded);
    StartAnimation(GetMovementComponent()->VaultMontageHigh, callBack, InPlayRate);
}

void ACharacterBase::StartAnimation(UAnimMontage *AnimMontage,
                                    const FOnMontageBlendingOutStarted &EndCallBack,
                                    float InPlayRate,
                                    FName StartSectionName)
{
    check(AnimMontage != nullptr && "Start Anim: null anim");
    PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
    auto animInstance    = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
    auto montageInstance = animInstance->GetActiveInstanceForMontage(AnimMontage);
    montageInstance->OnMontageBlendingOutStarted = EndCallBack;
}

void ACharacterBase::InterruptAnimation(ECCState Source, int OldState)
{
    //todo
    // already controlled
    if (OldState != ECCState::NoCC)
    {
        //check if highest order bit of source is lower than old
        if (GetHighestPriorityCC(Source) < GetHighestPriorityCC(OldState))
        {
            UE_LOG(LogTemp,
                   Warning,
                   TEXT("InterruptAnimation: cant interrupt CC, low priority"))
            return;
        }
    }
    else
    {
        ///// todo determine character state (combat, movement, transition, )
        // interrupted by character
        // player can interrupt blocks, spell casts, wall hanging, some attacks and not much else
        if (Source == ECCState::NoCC)
        {
            if (GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
            {
                auto animInstance =
                  Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
                if (animInstance->Montage_IsActive(GetMovementComponent()->HangMontage))
                    animInstance->StopSlotAnimation(0.f, FAnimSlotNames::FullBody);
            }
            return;
        }
        // interrupted by CC
        if (GetMovementComponent()->IsTransitionInProgress())
            GetMovementComponent()->TransitionInterrupt();
        // something else
    }
    CurrentCCState |= Source;
    FOnMontageBlendingOutStarted callBack;
    callBack.BindUObject(this,
                         CCInterruptDelegates[static_cast<ECCState>(
                           1 << GetHighestPriorityCC(CurrentCCState))]);
    StartAnimation(CCAnims[Source], callBack, 1.f); // play another instead
    // todo account for repeated CC application (stunLock)
}

void ACharacterBase::CCStunEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    // todo
    UE_LOG(LogTemp, Warning, TEXT("CCInterrupt: Stun"));
    CurrentCCState &= ~ECCState::Stun;
}

void ACharacterBase::CCKnockDownEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    // todo reset some armor debuffs
    UE_LOG(LogTemp, Warning, TEXT("CCInterrupt: KnockDown"));
    CurrentCCState &= ~ECCState::KnockDown;
}

void ACharacterBase::CCStatisEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    // todo reset invul
    UE_LOG(LogTemp, Warning, TEXT("CCInterrupt: Stasis"));
    CurrentCCState &= ~ECCState::Stasis;
}

int ACharacterBase::GetHighestPriorityCC(int bitmask)
{
#if PLATFORM_WINDOWS
    unsigned long index;
    _BitScanReverse(&index, bitmask);
    return index;
#else
    return ffs(bitmask);
#endif
}
