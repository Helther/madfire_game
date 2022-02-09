#include "CombatComponent.h"

#include "../CharacterCreature.h"
#include "Kismet/KismetMathLibrary.h"

const FName FAnimSectionNames::Start = "Start";
const FName FAnimSectionNames::End = "End";
const FName FAnimSectionNames::Cancel = "Cancel";


const FName FWeaponSlotNames::Main = "WeaponMainSlot";
const FName FWeaponSlotNames::Off = "WeaponOffSlot";
const FName FWeaponSlotNames::SheathMain = "WeaponSheathMainSlot";
const FName FWeaponSlotNames::SheathOff = "WeaponSheathOffSlot";
const FName FWeaponSlotNames::Back = "WeaponSheathBackSlot";

UCombatComponent::UCombatComponent(const class FObjectInitializer &ObjectInitilizer)
  : Super(ObjectInitilizer)
{
    PrimaryComponentTick.bCanEverTick = true;
    // TODO temp test
    AttackSpeed = 1.f;// TODO set in update from dex mod?
    UnarmedBaseDamage = 1.f;
    CastSpeed = 1.f;
    BlockStaminaRegenRateMod = 0.5f;
    ShowDebugInfo = true;
    CombatModeRotationRate = 500.f;
    ActionRotationRate = 100.f;
    ChargeStartTime = 0.2f;
    BlockStaminaCost = 10.f;
    KnockdownThreshhold = 1.5f;
    HitForceReductionMod = 0.8f;
    MaxFacingAngle = 45.f;
    // end temp

    InitCombatFSM();
}

FName UCombatComponent::GetSlotFromWeaponType(EWeaponType type, bool draw, bool mainHand)
{
    if (draw)
    {
        return mainHand ? FWeaponSlotNames::Main : FWeaponSlotNames::Off;
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
        case EWeaponType::Shield:
            return FWeaponSlotNames::Back;
        default:
            return mainHand ? FWeaponSlotNames::SheathMain : FWeaponSlotNames::SheathOff;
        }
    }
}

float UCombatComponent::CalculateWeaponDamage(const UItemWeapon* weapon, float attackInfoDamageMod, float strMod)
{
    if (weapon->WeaponType == EWeaponType::Unarmed)
    {
        auto gloves = Character->EquipmentComponent->GetEquipmentFromSlot(EItemSlot::HandsSlot)->Item;
        if (gloves)
            return (UnarmedBaseDamage + gloves->Weight) * strMod;
        else
            return UnarmedBaseDamage * strMod;
    }
    else
    {
        return weapon->DamageValue * attackInfoDamageMod + strMod;
    }
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    Character = Cast<ACharacterCreature>(GetOwner());
    check(Character != nullptr && "UCombatComponent: No pawn");
    Character->GetMovementComponent()->OnMoveModeChanged.AddDynamic(this, &UCombatComponent::OnMovementModeChanged);
    // TODO temp, stance resolve this on new stance
    GetCurrentCombatStance()->CurrentCrouchLocomotion = GetCurrentCombatStance()->CustomCrouchLocomotion;
    GetCurrentCombatStance()->CurrentStandLocomotion = GetCurrentCombatStance()->CustomStandLocomotion;
    GetCurrentCombatStance()->CurrentBlockAnim = GetCurrentCombatStance()->CustomBlockAnim;
    GetCurrentCombatStance()->CurrentParryAnim = GetCurrentCombatStance()->CustomParryAnim;
    GetCurrentCombatStance()->CurrentDrawAnim = GetCurrentCombatStance()->CustomDrawAnim;
    GetCurrentCombatStance()->CurrentSheathAnim = GetCurrentCombatStance()->CustomSheathAnim;
    /////
    // TODO temp weapon spawn, instead should spawn when setting in equipment
    CurrentWeaponMainH = DefaultWeaponMainH;
    auto actor = GetWorld()->SpawnActor(CurrentWeaponMainH->ItemActor);
    CurrentMainWeaponActor = Cast<AItemActor>(actor);
    CurrentMainWeaponActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FWeaponSlotNames::SheathMain);
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    auto currentState = CombatFSM.currentState();
    if (currentState && CurrentCombatStance && currentState->Id == ECombatState::Attack && AttackPressed)// check if charging
    {
        if (currentState->Info.CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed))
        {
            currentState->Info.ChargeTime += DeltaTime;
            auto &currentCombo = CurrentCombatStance->ComboAttacks[CurrentCombatStance->CurrentCombo];
            // check if held attack for at least some time to start charging and has charge stage
            if ((currentState->Info.ChargeStage == -1 && currentState->Info.ChargeTime < ChargeStartTime) ||
                currentCombo.ChargeAttackStages.Num() == 0 )
                return;
            // charged enough to go to next stage
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
                    Character->StartAnimation(currentState->Info.AttackInfo->AttackMontage, callBack, AttackSpeed, currentState->Info.AttackInfo->AnimSectionName);
                }
            }
        }
    }
}

void UCombatComponent::InitCombatFSM()
{
    CombatFSM.addState(ECombatState::Idle).addState(ECombatState::WeaponSwitch).addState(ECombatState::Attack).addState(ECombatState::Block).addState(ECombatState::Parry).addState(ECombatState::Ability);
    //=================================== IDLE =====================================//
    CombatFSM.state(ECombatState::Idle)->Info.OnEnter.BindLambda([&]()
    {
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("idle enter: %s"),
                IsWeaponDrawn ? TEXT("weapon on") : TEXT("weapon off"));
    });
    CombatFSM.state(ECombatState::Idle)->Info.OnLeave.BindLambda([&]()
    {
        CombatFSM.state(ECombatState::Idle)->Info.ResetAnimInfo();
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("idle leave"));
    });
    FLinkReq req;
    req.BindLambda([&]()
    {
        bool hasStanceAnim = CurrentCombatStance->CurrentDrawAnim != nullptr && CurrentCombatStance->CurrentSheathAnim != nullptr;
        return hasStanceAnim && CombatFSM.currentState()->Id == ECombatState::Idle && WeaponSwitchPressed;
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::WeaponSwitch, req);
    req.Unbind();
    req.BindLambda([&]() 
    {
        bool hasStamina = Character->AttributeComponent->Stamina->CurrentValue > 0;
        bool hasStanceAnims = CurrentCombatStance->CurrentBlockAnim != nullptr;
        return IsWeaponDrawn && hasStanceAnims && IsWeaponDrawn && CombatFSM.currentState()->Id == ECombatState::Idle && BlockPressed && hasStamina;
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::Block, req);
    req.Unbind();
    req.BindLambda([&]() // set relevant attack info into current state, check special, if not start combo
    {
        bool hasStamina = Character->AttributeComponent->Stamina->CurrentValue > 0;
        if (!IsWeaponDrawn || !AttackPressed || !hasStamina || Character->GetMovementComponent()->IsTransitionInProgress())
            return false;
        ESpecialType special = ESpecialType::None;
        if (Character->GetMovementComponent()->bIsSprinting)
            special = ESpecialType::Sprint;
        else if (Character->GetMovementComponent()->IsCrouching())
            special = ESpecialType::Crouch;
        else if (Character->GetMovementComponent()->IsFalling())
            special = ESpecialType::Air;
        else
        {
            if (SpecialDirections & ESpecialDirection::SFORWARD)
                special = ESpecialType::Forward;
            else if (SpecialDirections & ESpecialDirection::SBACK)
                special = ESpecialType::Back;
            else if (SpecialDirections & ESpecialDirection::SLEFT)
                special = ESpecialType::Left;
            else if (SpecialDirections & ESpecialDirection::SRIGHT)
                special = ESpecialType::Right;
        }
        if (special != ESpecialType::None)
            CombatFSM.state(ECombatState::Attack)->Info.AttackInfo = CurrentCombatStance->GetAttackInfoForSpecial(special);
        else
            CombatFSM.state(ECombatState::Attack)->Info.AttackInfo = &CurrentCombatStance->ComboAttacks[0].NormalAttack;
        return CombatFSM.state(ECombatState::Attack)->Info.AttackInfo != nullptr && CombatFSM.state(ECombatState::Attack)->Info.AttackInfo->AttackMontage != nullptr;
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::Attack, req);
    req.Unbind();
    req.BindLambda([&]() // check if not active and not in cooldown and more
    {
        bool hasStamina = Character->AttributeComponent->Stamina->CurrentValue > 0;
        bool globalCool = GetWorld()->GetTimerManager().IsTimerActive(GlobalCooldownTimer);
        return AbilityPressed && hasStamina && CurrentAbility != nullptr && !CurrentAbility->bIsCooldown && !globalCool;
    });
    CombatFSM.addLink(ECombatState::Idle, ECombatState::Ability, req);
    req.Unbind();

    //================================ WEAPON SWITCH ==============================//
    CombatFSM.state(ECombatState::WeaponSwitch)->Info.OnEnter.BindLambda([&]()
    {// TODO check stance resolve and set current anims
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("weapon switch enter"));
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(this, &UCombatComponent::OnWeaponSwitchEnded);
        Character->StartAnimation(IsWeaponDrawn ? 
            CurrentCombatStance->CurrentSheathAnim : CurrentCombatStance->CurrentDrawAnim, callBack, AttackSpeed);
    });
    CombatFSM.state(ECombatState::WeaponSwitch)->Info.OnLeave.BindLambda([&]()
    {
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("weapon switch leave"));
        if (IsWeaponDrawn)// in combat mode
        {
            Character->GetMovementComponent()->maxSpeedBlock = true;
            CalculateMaxCombatSpeeds();
            Character->GetMovementComponent()->bUseControllerDesiredRotation = true;
            Character->GetMovementComponent()->bOrientRotationToMovement = false;
            Character->GetMovementComponent()->RotationRate.Yaw = CombatModeRotationRate;
        }
        else// exit combat mode
        {// reset max speeds
            Character->GetMovementComponent()->maxSpeedBlock = false;
            Character->GetMovementComponent()->CalculateMaxSpeeds(Character->GetMovementComponent()->GetSpeedModifier());
            Character->GetMovementComponent()->bUseControllerDesiredRotation = false;
            Character->GetMovementComponent()->bOrientRotationToMovement = true;
        }
        CombatFSM.state(ECombatState::WeaponSwitch)->Info.ResetAnimInfo();
    });
    req.BindLambda([&]()  // to idle after animation complete
    {
        return CombatFSM.state(ECombatState::WeaponSwitch)->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded); 
    });
    CombatFSM.addLink(ECombatState::WeaponSwitch, ECombatState::Idle, req);
    req.Unbind();

    //=================================== Block =====================================//
    CombatFSM.state(ECombatState::Block)->Info.OnEnter.BindLambda([&]()
    {
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("block enter"));
        CalculateMaxBlockingSpeed(Character->GetMovementComponent()->GetSpeedModifier());
        Character->AttributeComponent->Stamina->RegenCoef += BlockStaminaRegenRateMod; // slowing down stamina regen rate
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(this, &UCombatComponent::OnBlockEnded);
        Character->StartAnimation(CurrentCombatStance->CurrentBlockAnim, callBack, AttackSpeed, FAnimSectionNames::Start);
    });
    CombatFSM.state(ECombatState::Block)->Info.OnLeave.BindLambda([&]()
    {
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("block leave"));
        CalculateMaxCombatSpeeds();
        Character->AttributeComponent->Stamina->RegenCoef -= BlockStaminaRegenRateMod; // reseting stamina regen rate
        CombatFSM.state(ECombatState::Block)->Info.ResetAnimInfo();
    });
    req.BindLambda([&]() // to idle after animation complete
    {
        const auto state = CombatFSM.state(ECombatState::Block);
        return state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && 
            !state->Info.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt); 
    });
    CombatFSM.addLink(ECombatState::Block, ECombatState::Idle, req);
    req.Unbind();
    req.BindLambda([&]() // check if stance has parry (Note: кулачный бой не должен иметь парри)
    {
        bool hasStanceAnims = CurrentCombatStance->CurrentParryAnim != nullptr;
        bool hasStamina = Character->AttributeComponent->Stamina->CurrentValue > 0;
        const auto state = CombatFSM.state(ECombatState::Block);
        return hasStanceAnims && state->Info.CheckAnimInfo(ECombatStateAnimInfo::BlockOrParryActive) && AttackPressed && hasStamina; 
    });
    CombatFSM.addLink(ECombatState::Block, ECombatState::Parry, req);
    req.Unbind();
    
    //=================================== Parry =====================================//
    CombatFSM.state(ECombatState::Parry)->Info.OnEnter.BindLambda([&]()
    {
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("parry enter"));
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(this, &UCombatComponent::OnParryEnded);
        Character->StartAnimation(CurrentCombatStance->CurrentParryAnim, callBack, AttackSpeed);
        Character->AttributeComponent->Stamina->Consume(CurrentCombatStance->ParryStaminaCost);
        Character->AttributeComponent->Stamina->BlockRegen();
        Character->GetMovementComponent()->RotationRate.Yaw = ActionRotationRate;
    });
    CombatFSM.state(ECombatState::Parry)->Info.OnLeave.BindLambda([&]()
    {
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("parry leave"));
        ResetActionState(ECombatState::Parry);
    });
    req.BindLambda([&]()  // to idle after animation complete
    {
        const auto state = CombatFSM.state(ECombatState::Parry);
        return state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && !BlockPressed;
    });
    CombatFSM.addLink(ECombatState::Parry, ECombatState::Idle, req);
    req.Unbind();
    req.BindLambda([&]()  // to block after animation complete and blocked or manual interrupt
    {
        bool hasStanceAnims = CurrentCombatStance->CurrentBlockAnim != nullptr;
        bool hasStamina = Character->AttributeComponent->Stamina->CurrentValue > 0; // can't start block without stamina
        const auto state = CombatFSM.state(ECombatState::Parry);
        bool checkEnd = state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && BlockPressed;
        return hasStamina && hasStanceAnims && (checkEnd || CanBeInterruptedWithBlock(ECombatState::Parry)) && !Character->GetMovementComponent()->IsRootOrTeleport();
    });
    CombatFSM.addLink(ECombatState::Parry, ECombatState::Block, req);
    req.Unbind();
    req.BindLambda([&]()// TODO finish counter attack check, check if was parried (no special counter atttacks planned at the moment)
    {
        auto newAttackInfo = CurrentCombatStance->GetAttackInfoForSpecial(ESpecialType::CounterAttack);
        CombatFSM.state(ECombatState::Attack)->Info.AttackInfo = newAttackInfo;
        return newAttackInfo != nullptr && 0;// TODO temp noop
    });
    CombatFSM.addLink(ECombatState::Parry, ECombatState::Attack, req);
    req.Unbind();
    
    //=================================== Attack ====================================//
    CombatFSM.state(ECombatState::Attack)->Info.OnEnter.BindLambda([&]()
    {
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("attack enter"));
        const FAttackInfo* currentAttack = CombatFSM.state(ECombatState::Attack)->Info.AttackInfo;
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
        Character->StartAnimation(currentAttack->AttackMontage, callBack, AttackSpeed, currentAttack->AnimSectionName);
        bool isComboAttack = CombatFSM.state(ECombatState::Attack)->Info.AttackInfo->Special == ESpecialType::None;
        bool hasChargeAttack = CurrentCombatStance->ComboAttacks[0].ChargeAttackStages.Num() > 0;
        if (isComboAttack && hasChargeAttack)
            CombatFSM.state(ECombatState::Attack)->Info.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, true);
        Character->AttributeComponent->Stamina->Consume(currentAttack->StaminaCost);
        Character->AttributeComponent->Stamina->BlockRegen();
        Character->GetMovementComponent()->RotationRate.Yaw = ActionRotationRate;
        CalculateMaxBlockingSpeed(Character->GetMovementComponent()->GetSpeedModifier());
    });
    CombatFSM.state(ECombatState::Attack)->Info.OnLeave.BindLambda([&]()
    {
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("attack leave"));
        ResetActionState(ECombatState::Attack);
        CombatFSM.state(ECombatState::Attack)->Info.ChargeTime = 0.f;
        CombatFSM.state(ECombatState::Attack)->Info.AttackInfo = nullptr;
        CombatFSM.state(ECombatState::Attack)->Info.ChargeStage = -1;
        ResetCurrentCombo();
        CharactersHit.Reset();
        SetCurrentWeaponCollision(true, false);
        SetCurrentWeaponCollision(false, false);
        CalculateMaxCombatSpeeds();
    });
    req.BindLambda([&]()
    {
        const auto state = CombatFSM.state(ECombatState::Attack);
        return state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && !BlockPressed &&
            !state->Info.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt);
    });
    CombatFSM.addLink(ECombatState::Attack, ECombatState::Idle, req);
    req.Unbind();
    req.BindLambda([&]() // manual interrupt
    {
        bool hasStanceAnims = CurrentCombatStance->CurrentBlockAnim != nullptr;
        bool hasStamina = Character->AttributeComponent->Stamina->CurrentValue > 0;        
        const auto state = CombatFSM.state(ECombatState::Attack);
        bool checkEnd = state->Info.CheckAnimInfo(ECombatStateAnimInfo::AnimEnded) && BlockPressed &&
            !state->Info.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt);
        return hasStamina && hasStanceAnims && (checkEnd || CanBeInterruptedWithBlock(ECombatState::Attack));
    });
    CombatFSM.addLink(ECombatState::Attack, ECombatState::Block, req);
    req.Unbind();

    //=================================== Ability ====================================//
    CombatFSM.state(ECombatState::Ability)->Info.OnEnter.BindLambda([&]()
    {
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("Ability enter"));
        if (!IsWeaponDrawn)// not in combat mode
        {
            Character->GetMovementComponent()->maxSpeedBlock = true;
            Character->GetMovementComponent()->bUseControllerDesiredRotation = true;
            Character->GetMovementComponent()->bOrientRotationToMovement = false;
        }
        CalculateMaxCastingSpeed(Character->GetMovementComponent()->GetSpeedModifier());
        Character->AttributeComponent->Stamina->Consume(CurrentAbility->StaminaCost);
        Character->AttributeComponent->Stamina->BlockRegen();
        Character->GetMovementComponent()->MoveAbilitiesCurrent = CurrentAbility->MoveAbilities;
        CurrentAbility->ActivateAbility();
    });
    CombatFSM.state(ECombatState::Ability)->Info.OnLeave.BindLambda([&]()
    {
        if (ShowDebugInfo)
            UE_LOG(LogTemp, Warning, TEXT("Ability leave"));
        if (IsWeaponDrawn)// in combat mode
            CalculateMaxCombatSpeeds();
        else
        {
            Character->GetMovementComponent()->maxSpeedBlock = false;
            Character->GetMovementComponent()->CalculateMaxSpeeds(Character->GetMovementComponent()->GetSpeedModifier());
            Character->GetMovementComponent()->bUseControllerDesiredRotation = false;
            Character->GetMovementComponent()->bOrientRotationToMovement = true;
        }
        Character->AttributeComponent->Stamina->UnblockRegen();
        Character->GetMovementComponent()->MoveAbilitiesCurrent = Character->GetMovementComponent()->MoveAbilitiesDefault;// TODO think how to restore it to last state
        GetWorld()->GetTimerManager().SetTimer(GlobalCooldownTimer, GlobalCooldownTime, false);
    });
    req.BindLambda([&]()
    {
        return CurrentAbility != nullptr && !CurrentAbility->bIsActive;
    });
    CombatFSM.addLink(ECombatState::Ability, ECombatState::Idle, req);
    req.Unbind();

    CombatFSM.setCurrentState(ECombatState::Idle);
}

bool UCombatComponent::UpdateCombatFSM()
{
    int oldState = CombatFSM.currentState()->Id;
    auto newState = CombatFSM.proccessNextState();
    if (newState != nullptr)
    {
        CombatFSM.setCurrentState(newState->Id);
        OnCombatStateChanged.Broadcast(oldState,newState->Id);
    }
    return newState != nullptr;
}

void UCombatComponent::ResetActionState(int StateId)
{
    // reset root motion ignore
    auto animInstance = Cast<UCharacterAnimInstance>(Character->GetMesh()->GetAnimInstance());
    animInstance->OnMovementAllowed(true);
    animInstance->OnRotationAllowed(false);
    Character->AttributeComponent->Stamina->UnblockRegen();
    CombatFSM.state(StateId)->Info.ResetAnimInfo();
    Character->GetMovementComponent()->RotationRate.Yaw = CombatModeRotationRate;
}

void UCombatComponent::OnMovementModeChanged(int oldMode, int newMode)
{ 
    if (CombatFSM.currentState() && CombatFSM.currentState()->Id == ECombatState::Attack)
    {
        // check if started falling during regular attack
        if (newMode == EMovementMode::MOVE_Falling && !CombatFSM.currentState()->Info.CheckAnimInfo(ECombatStateAnimInfo::ChargeAllowed))
            Character->InterruptAnimation(ECCState::NoCC);
        // check if landed during air attack to finish anim
        if (newMode == EMovementMode::MOVE_Walking && CombatFSM.currentState()->Info.AttackInfo->Special == ESpecialType::Air)
        {
            CombatFSM.currentState()->Info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
            FOnMontageBlendingOutStarted callBack;
            callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
            Character->StartAnimation(CombatFSM.currentState()->Info.AttackInfo->AttackMontage, callBack, AttackSpeed, FAnimSectionNames::End);	
        }
    }
}

void UCombatComponent::OnWeaponOverlap(UPrimitiveComponent* comp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    auto hitActor = SweepResult.GetActor();
    if (hitActor == nullptr || hitActor == Character || hitActor == CurrentMainWeaponActor || hitActor == CurrentOffWeaponActor)// Check self and weapon
        return;
    if (CombatFSM.currentState()->Id == ECombatState::Attack)
    {
        const auto &info = CombatFSM.state(ECombatState::Attack)->Info;
        if (info.CheckAnimInfo(ECombatStateAnimInfo::CollisionOn) && !HasAlreadyHitTarget(hitActor))
        {
            CharactersHit.Emplace(hitActor);
            // check witch hand has active collision
            auto activeWeapon = CurrentMainWeaponActor->ItemCollision->GetGenerateOverlapEvents() ? CurrentWeaponMainH : CurrentWeaponOffH;
            auto hitCharacter = Cast<ACharacterBase>(hitActor);
            if (hitCharacter)
            {
                if (ShowDebugInfo)
                    UE_LOG(LogTemp, Warning, TEXT("OnBeginOverlap Hit character!"));
                int32 strMod = UAttributeSetHelper::GetModifier(hitCharacter->AttributeComponent->Attributes, EAttributes::Str);
                const float damage = CalculateWeaponDamage(activeWeapon, CombatFSM.state(ECombatState::Attack)->Info.AttackInfo->DamageMod, strMod);
                float force = CalculateHitForce(activeWeapon->Weight, strMod, info.AttackInfo->PowerMod); // Use Force, Luke
                hitCharacter->TakeDamage(damage, FForceDamageEvent(activeWeapon->DamageType, force), Character->GetController(), Character);
            }
            else // hit an obsticle
            {
                if (SweepResult.GetComponent()->GetCollisionObjectType() == ECollisionChannel::ECC_WorldStatic) // check if hit a wall and interrupt with loss of balance
                {
                    if (ShowDebugInfo)
                        UE_LOG(LogTemp, Warning, TEXT("OnBeginOverlap Hit wall!"));
                    Character->InterruptAnimation(ECCState::LossOfBalance, ELossOfBalanceCCSubState::HitWall);
                }
                else // try to damage the actor
                {
                    if (ShowDebugInfo)
                        UE_LOG(LogTemp, Warning, TEXT("OnBeginOverlap Hit actor!"));
                    const float damage = CalculateWeaponDamage(activeWeapon, CombatFSM.state(ECombatState::Attack)->Info.AttackInfo->DamageMod, 0);
                    hitActor->TakeDamage(damage, FDamageEvent(activeWeapon->DamageType), Character->GetController(), Character);
                }
            }
        }
    }
}

void UCombatComponent::Update()
{
    // TODO check stance validity, call stanceresolver, update speeds if needed
    UnSetUpdate();
}

void UCombatComponent::OnAttack(bool Started)
{
    if (ShowDebugInfo)
        UE_LOG(LogTemp, Warning, TEXT("OnAttack %s"),
            Started ? TEXT("pressed") : TEXT("released"));
    AttackPressed = Started;
    if (CombatFSM.currentState()->Id == ECombatState::Attack)
    {
        auto &info = CombatFSM.state(ECombatState::Attack)->Info;
        const FAttackInfo* currentAttack = info.AttackInfo;
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
                    bool hasStamina = Character->AttributeComponent->Stamina->CurrentValue > 0;
                    if (Character->GetMovementComponent()->MovementMode == EMovementMode::MOVE_Falling)// uncharging in air
                    {
                        auto airAttack = CurrentCombatStance->GetAttackInfoForSpecial(ESpecialType::Air);
                        if (hasStamina && airAttack != nullptr)
                        {
                            info.AttackInfo = CurrentCombatStance->GetAttackInfoForSpecial(ESpecialType::Air);
                            info.ResetAnimInfo();
                            info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                            Character->AttributeComponent->Stamina->Consume(info.AttackInfo->StaminaCost);
                            FOnMontageBlendingOutStarted callBack;
                            callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
                            Character->StartAnimation(info.AttackInfo->AttackMontage, callBack, AttackSpeed);
                        }
                        return;
                    }
                    else if (hasStamina && currentAttack->AttackMontage && info.ChargeTime >=
                        CurrentCombatStance->ComboAttacks[CurrentCombatStance->CurrentCombo].ChargeAttackStages[CombatFSM.currentState()->Info.ChargeStage].ChargeTimeNeeded)
                    {// uncharging on ground
                        Character->AttributeComponent->Stamina->Consume(currentAttack->StaminaCost);
                        info.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, false);
                        info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                        FOnMontageBlendingOutStarted callBack;
                        callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
                        Character->StartAnimation(currentAttack->AttackMontage, callBack, AttackSpeed, FAnimSectionNames::End);
                        return;
                    }
                    // charging cancelled
                    info.ChangeAnimInfo(ECombatStateAnimInfo::ChargeAllowed, false);
                    info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
                    FOnMontageBlendingOutStarted callBack;
                    callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
                    Character->StartAnimation(currentAttack->AttackMontage, callBack, AttackSpeed, FAnimSectionNames::Cancel);
                }
            }
        }
    }
    else if (AttackPressed)// starting first attack
        UpdateCombatFSM();
}

void UCombatComponent::OnBlock(bool Started) 
{
    if (ShowDebugInfo)
        UE_LOG(LogTemp, Warning, TEXT("OnBlock %s"),
            Started ? TEXT("pressed") : TEXT("released"));
    BlockPressed = Started;
    if (!BlockPressed && CombatFSM.currentState()->Id == ECombatState::Block) // end block
    {
        CombatFSM.currentState()->Info.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(this, &UCombatComponent::OnBlockEnded);
        Character->StartAnimation(CurrentCombatStance->CurrentBlockAnim, callBack, AttackSpeed, FAnimSectionNames::End);
    }
    else
        UpdateCombatFSM();
}

void UCombatComponent::OnDrawWeapon()
{
    WeaponSwitchPressed = true;
    UpdateCombatFSM();
    WeaponSwitchPressed = false;
}

void UCombatComponent::OnUseAbility(bool Started)
{
    AbilityPressed = Started;
    if (CurrentAbility != nullptr)
    {
        if (AbilityPressed)
        {
            if (!CurrentAbility->bIsActive)
            {
                UpdateCombatFSM();
            }
        }
        else
        {
            if (CurrentAbility->bIsActive)
            {
                CurrentAbility->ReleaseAbility();
            }
        }
    }
}

void UCombatComponent::StartNextComboAttack(bool IsPauseAttack)
{
    check(CombatFSM.currentState()->Id == ECombatState::Attack && "StartNextComboAttack: not in attack state");
    check(CurrentCombatStance && "StartNextComboAttack: doesn't have stance");
    if (CurrentCombatStance->CurrentCombo >= CurrentCombatStance->ComboAttacks.Num() - 1)
        return;
    if (Character->AttributeComponent->Stamina->CurrentValue <= 0)
        return;

    auto& stateInfo = CombatFSM.currentState()->Info;
    FAttackInfo* nextAttackInfo = nullptr;
    if (IsPauseAttack)
    {
        if (CurrentCombatStance->ComboAttacks[CurrentCombatStance->CurrentCombo + 1].PauseAttack.AttackMontage != nullptr)
        {
            stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::PauseComboSaved, true);
            nextAttackInfo = &CurrentCombatStance->ComboAttacks[CurrentCombatStance->CurrentCombo + 1].PauseAttack;
        }
        else
            return;
    }
    else
    {
        if (stateInfo.CheckAnimInfo(ECombatStateAnimInfo::ComboSaved))
        {
            if (CurrentCombatStance->ComboAttacks[CurrentCombatStance->CurrentCombo + 1].NormalAttack.AttackMontage != nullptr)
                nextAttackInfo = &CurrentCombatStance->ComboAttacks[CurrentCombatStance->CurrentCombo + 1].NormalAttack;
        }
        else
            return;
    }
    if (nextAttackInfo != nullptr)
    {
        ++CurrentCombatStance->CurrentCombo;
        Character->AttributeComponent->Stamina->Consume(nextAttackInfo->StaminaCost);
        stateInfo.ResetAnimInfo();
        CharactersHit.Reset();
        SetCurrentWeaponCollision(true, false);
        SetCurrentWeaponCollision(false, false);
        auto animInstance = Cast<UCharacterAnimInstance>(Character->GetMesh()->GetAnimInstance());
        animInstance->OnMovementAllowed(false);
        stateInfo.AttackInfo = nextAttackInfo;
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, true);
        FOnMontageBlendingOutStarted callBack;
        callBack.BindUObject(this, &UCombatComponent::OnAttackEnded);
        Character->StartAnimation(stateInfo.AttackInfo->AttackMontage, callBack, AttackSpeed, stateInfo.AttackInfo->AnimSectionName);
    }
}

bool UCombatComponent::HasAlreadyHitTarget(AActor *Target) const
{
    if (Target)
        return CharactersHit.Contains(Target);
    return false;
}

float UCombatComponent::GetAngleBetweenAttackerAndDefender(AActor *Attacker, AActor *Defender)
{
    check(Attacker != nullptr && Defender != nullptr && "GetAngleBetweenActors: invalid arg");
    const FVector defenderV = FRotator(0, Defender->GetActorRotation().Yaw, 0).Vector();
    const FVector attackerV = UKismetMathLibrary::GetDirectionUnitVector(Attacker->GetActorLocation(), Defender->GetActorLocation());
    float direction = FVector::DotProduct(attackerV, defenderV);
    return FMath::Abs(acos(direction) * 180.f / PI - 180.f);
}

float UCombatComponent::GetAngleBetweenActors(AActor *First, AActor *Second)
{
    check(First != nullptr && Second != nullptr && "GetAngleBetweenActors: invalid arg");
    const FVector firstV = FRotator(0, First->GetActorRotation().Yaw, 0).Vector();
    const FVector secondV = FRotator(0, Second->GetActorRotation().Yaw, 0).Vector();
    float direction = FVector::DotProduct(firstV, secondV);
    return FMath::Abs(acos(direction) * 180.f / PI - 180.f);
}

void UCombatComponent::OnWeaponSwitchEnded(UAnimMontage* animMontage, bool bInterrupted)
{
    auto& stateInfo = CombatFSM.state(ECombatState::WeaponSwitch)->Info;
    if (!bInterrupted || stateInfo.CheckAnimInfo(ECombatStateAnimInfo::WeaponGrabbed))
        IsWeaponDrawn = !IsWeaponDrawn;
    stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
    UpdateCombatFSM();
}

void UCombatComponent::OnBlockEnded(UAnimMontage* animMontage, bool bInterrupted)
{
    if (bInterrupted && CombatFSM.currentState()->Id == ECombatState::Parry)// interrupted by Parry
        return;
    auto& stateInfo = CombatFSM.state(ECombatState::Block)->Info;
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
    auto& stateInfo = CombatFSM.state(ECombatState::Parry)->Info;
    if (CombatFSM.currentState()->Id == ECombatState::Parry)
    {
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
        UpdateCombatFSM();
    }
}

void UCombatComponent::OnAttackEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    if (CombatFSM.currentState()->Id != ECombatState::Attack)// interrupted
        return;
    auto& stateInfo = CombatFSM.state(ECombatState::Attack)->Info;
    if (stateInfo.CheckAnimInfo(ECombatStateAnimInfo::UserInterrupt))
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::UserInterrupt, false);// continued combo
    else
    {
        stateInfo.ChangeAnimInfo(ECombatStateAnimInfo::AnimEnded, true);
        UpdateCombatFSM();
    }
}

UCombatStance *UCombatComponent::GetCurrentCombatStance() const
{
    check(CurrentCombatStance != nullptr && "Combat component: no combat stance");
    return CurrentCombatStance;
}

void UCombatComponent::OnStanceChanged()
{
    if (IsWeaponDrawn)    // TODO call update
        CalculateMaxCombatSpeeds();
}

void UCombatComponent::CalculateMaxCombatSpeeds()
{
    float speedMod = Character->GetMovementComponent()->GetSpeedModifier();
    Character->GetMovementComponent()->MaxRunSpeed =
        CurrentCombatStance->MaxCombatRunSpeed * speedMod;
    Character->GetMovementComponent()->MaxSprintSpeed =
        CurrentCombatStance->MaxCombatSprintSpeed * speedMod;
}

void UCombatComponent::CalculateMaxBlockingSpeed(float SpeedModifier)
{
    Character->GetMovementComponent()->MaxRunSpeed = CurrentCombatStance->MaxBlockSpeed * SpeedModifier;
}

bool UCombatComponent::CanBeInterruptedWithBlock(int CurrentState) const
{
    const auto state = CombatFSM.state(CurrentState);
    bool pass = !state->Info.CheckAnimInfo(ECombatStateAnimInfo::MovementBlocked) && BlockPressed;
    return pass;
}

void UCombatComponent::WeaponsSetup(bool draw)
{
    if (CurrentWeaponMainH != nullptr)
    {
        WeaponSetup(CurrentWeaponMainH, CurrentMainWeaponActor, draw, true);
        if (CurrentWeaponOffH != nullptr)
            WeaponSetup(CurrentWeaponOffH, CurrentOffWeaponActor, draw, false);
    }
    else if (DefaultWeaponMainH != nullptr)
    {
        WeaponSetup(DefaultWeaponMainH, CurrentMainWeaponActor, draw, true);
        if (DefaultWeaponOffH != nullptr)
            WeaponSetup(DefaultWeaponOffH, CurrentOffWeaponActor, draw, false);
    }
}

void UCombatComponent::WeaponSetup(UItemWeapon* weapon, AItemActor* actor, bool draw, bool mainHand)
{
    const FName slot = UCombatComponent::GetSlotFromWeaponType(weapon->WeaponType, draw, mainHand);
    if (draw)
    {
        actor->ItemCollision->OnComponentBeginOverlap.AddDynamic(this, &UCombatComponent::OnWeaponOverlap);
        actor->ItemCollision->SetCollisionProfileName("OverlapAll");
    }
    else
    {
        actor->ItemCollision->OnComponentBeginOverlap.RemoveDynamic(this, &UCombatComponent::OnWeaponOverlap);
    }
    actor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, slot);
}

void UCombatComponent::SetCurrentWeaponCollision(bool main, bool enable)
{
    if (main && CurrentMainWeaponActor != nullptr)
    {
        CurrentMainWeaponActor->ItemCollision->SetGenerateOverlapEvents(enable);
        // other options
        //SetCollisionObjectType
        //SetCollisionResponseToChannel
    }
    else if (CurrentOffWeaponActor != nullptr)
    {
        CurrentOffWeaponActor->ItemCollision->SetGenerateOverlapEvents(enable);
    }
}

void UCombatComponent::ActivateAbility()
{
    if(CurrentAbility->IsValidLowLevelFast())
    {
        CurrentAbility->ActivateAbility();
    }
}

void UCombatComponent::ReleaseAbility()
{
    if(CurrentAbility->IsValidLowLevelFast())
    {
        CurrentAbility->ReleaseAbility();
    }
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
    CurrentAbility = Ability;
    if(CurrentAbility)
        CurrentAbility->AbilitySetup(this);
    else
    {
        if (ShowDebugInfo)
            UE_LOG(AbilitySystemLog, Error, TEXT("AbilitySystem Component: Ability Is Null"));
    }
}

void UCombatComponent::CalculateMaxCastingSpeed(float SpeedModifier)
{
    Character->GetMovementComponent()->MaxRunSpeed = MaxSpeedWhileCasting * SpeedModifier;
}
