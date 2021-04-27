// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterPlayer.h"

#include "PlayerCharController.h"

ACharacterPlayer::ACharacterPlayer(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    CombatComponent = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
    CurrentCCState = ECCState::NoCC;
    CCInterruptDelegates.Emplace(ECCState::Sleep, &ACharacterPlayer::CCStunEnded);
    CCInterruptDelegates.Emplace(ECCState::LossOfBalance, &ACharacterPlayer::CCStunEnded);
    CCInterruptDelegates.Emplace(ECCState::Stun, &ACharacterPlayer::CCStunEnded);
    CCInterruptDelegates.Emplace(ECCState::KnockDown, &ACharacterPlayer::CCKnockDownEnded);
    CCInterruptDelegates.Emplace(ECCState::Stasis, &ACharacterPlayer::CCStatisEnded);
}

// Called to bind functionality to input
void ACharacterPlayer::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACharacterPlayer::OnJumped_Implementation()
{
    GetMovementComponent()->bCanHangAgain   = false;
    GetMovementComponent()->bWasJustJumping = true;
}

void ACharacterPlayer::OnWalkingOffLedge_Implementation(
  const FVector &PreviousFloorImpactNormal,
  const FVector &PreviousFloorContactNormal,
  const FVector &PreviousLocation,
  float TimeDelta)
{
    UE_LOG(LogTemp, Warning, TEXT("OnWalkingOffLedge"));
    APlayerCharController *PlayerController =
      Cast<APlayerCharController>(GetController());
    if (PlayerController->IsActionKeyPressed(FInputKeyNames::ClimbDownTest))
    {
        UE_LOG(LogTemp, Warning, TEXT("climb down"));
        GetMovementComponent()->ClimbDownWall();
    }
}

void ACharacterPlayer::StartAnimTest(float InPlayRate)
{
    FOnMontageBlendingOutStarted callBack;
    callBack.BindUObject(GetMovementComponent(), &UBaseMovementComponent::OnParkourEnded);
    StartAnimation(GetMovementComponent()->VaultMontageHigh, callBack, InPlayRate);
}

void ACharacterPlayer::StartAnimation(UAnimMontage *AnimMontage,
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

void ACharacterPlayer::InterruptAnimation(ECCState Source, int OldState)
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

void ACharacterPlayer::CCStunEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    // todo
    UE_LOG(LogTemp, Warning, TEXT("CCInterrupt: Stun"));
    CurrentCCState &= ~ECCState::Stun;
}

UBaseMovementComponent *ACharacterPlayer::GetMovementComponent() const
{
    return Cast<UBaseMovementComponent>(ACharacter::GetMovementComponent());
}

void ACharacterPlayer::Update() 
{
    Super::Update();
    if (GetMovementComponent()->IsUpdateNeeded())
        GetMovementComponent()->Update();
}

void ACharacterPlayer::DoLevelUp(const FLevelInfo &levelInfo)
{
    bool dexChanged = UAttributesHelper::GetAttribute(levelInfo.Attributes, EAttributes::Dex) != 0;
    bool strChanged = UAttributesHelper::GetAttribute(levelInfo.Attributes, EAttributes::Str) != 0;
    bool acrobaticsChanged = USkillHelper::GetValue(levelInfo.SkillList, ESkill::Tumble) != 0;
    if (dexChanged || strChanged || acrobaticsChanged)
        GetMovementComponent()->SetUpdate();
    Super::DoLevelUp(levelInfo);
    // todo other stuff
}

void ACharacterPlayer::CCKnockDownEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    // todo reset some armor debuffs
    UE_LOG(LogTemp, Warning, TEXT("CCInterrupt: KnockDown"));
    CurrentCCState &= ~ECCState::KnockDown;
}

void ACharacterPlayer::CCStatisEnded(UAnimMontage *animMontage, bool bInterrupted)
{
    // todo reset invul
    UE_LOG(LogTemp, Warning, TEXT("CCInterrupt: Stasis"));
    CurrentCCState &= ~ECCState::Stasis;
}

void ACharacterPlayer::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    auto animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
    CCAnims.Emplace(ECCState::KnockDown, animInstance->KnockDownAnim);
    CCAnims.Emplace(ECCState::LossOfBalance, animInstance->LossOfBalanceAnim);
    CCAnims.Emplace(ECCState::Sleep, animInstance->SleepAnim);
    CCAnims.Emplace(ECCState::Stasis, animInstance->StasisAnim);
    CCAnims.Emplace(ECCState::Stun, animInstance->StunAnim);
}


int ACharacterPlayer::GetHighestPriorityCC(int bitmask)
{
#if PLATFORM_WINDOWS
    unsigned long index;
    _BitScanReverse(&index, bitmask);
    return index;
#else
    return ffs(bitmask);
#endif
}
