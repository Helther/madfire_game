// Fill out your copyright notice in the Description page of Project Settings.

#include "CCComponent.h"
#include "GameFramework/Character.h"


UCCComponent::UCCComponent(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    CCLockTimeDivider = 2.f;
    CCLockTime = 10.f;
    CurrentCCState = ECCState::NoCC;
}

// Called when the game starts
void UCCComponent::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void UCCComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCCComponent::SetCCMode(ECCState Mode, unsigned int SubMode, float *Time)
{
    GetOwner()->GetWorld()->GetTimerManager().ClearTimer(CCLocks[Mode].Key.StunLockTimer);
    FTimerDelegate lockDelegate = FTimerDelegate::CreateLambda([this, Mode]() { ResetCCLock(Mode); });
    GetOwner()->GetWorld()->GetTimerManager().SetTimer(CCLocks[Mode].Key.StunLockTimer, lockDelegate, CCLockTime, false);
    if (Time != nullptr)
    {
        float duration = *Time * GetCCLockTimeModifier(Mode);
        FTimerDelegate CCDelegate = FTimerDelegate::CreateLambda([this, Mode]() 
            {
                auto CharacterOwner = Cast<ACharacter>(GetOwner());
                check(CharacterOwner && CharacterOwner->GetMesh());
                auto AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
                check(AnimInstance);
                if (AnimInstance->Montage_IsActive(CCAnims[Mode]))
                    AnimInstance->Montage_Stop(0.f, CCAnims[Mode]);
            });
        GetOwner()->GetWorld()->GetTimerManager().SetTimer(CCLocks[Mode].Key.CCTimer, CCDelegate, duration, false);
    }
    CCLocks[Mode].Value++;
    CurrentCCState |= 1u << Mode;
    CurrentCCSubState |= 1u << SubMode;
    OnCCStateChanged.Broadcast(Mode, SubMode);
}

void UCCComponent::ResetCCMode(ECCState Mode)
{
    if (IsInCCMode(Mode))
    {
        CurrentCCState &= ~(1u << Mode);
        CurrentCCSubState = 0u;
        FTimerDelegate lockDelegate = FTimerDelegate::CreateLambda([this, Mode]() { ResetCCLock(Mode); });
        GetOwner()->GetWorld()->GetTimerManager().SetTimer(CCLocks[Mode].Key.StunLockTimer, lockDelegate, CCLockTime, false );
        OnCCStateChanged.Broadcast(0u, 0u);
    }
}

float UCCComponent::GetCCLockTimeModifier(ECCState Mode) const
{
    return CCLocks[Mode].Value ? 1.f / (CCLocks[Mode].Value * CCLockTimeDivider) : 1.f;
}

void UCCComponent::ResetCCLock(int index)
{
    CCLocks[index].Value = 0;
}

int UCCComponent::GetHighestPriorityCC(int bitmask)
{
#if PLATFORM_WINDOWS
    unsigned long index;
    _BitScanReverse(&index, bitmask);
    return index;
#else
    return ffs(bitmask)-1;
#endif
}
