// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"

#include "../CharacterPlayer.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent(const class FObjectInitializer &ObjectInitilizer)
  : Super(ObjectInitilizer)
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    Character = Cast<ACharacterPlayer>(GetOwner());    //для игрока каст в игрового перса
    check(Character != nullptr && "UBaseMovementComponent: No pawn");
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::Update()
{
    //
    UnSetUpdate();
}

void UCombatComponent::DoAttack() {}

void UCombatComponent::DoBlock() {}

void UCombatComponent::DrawWeapon() {}

void UCombatComponent::SheathWeapon() {}

void UCombatComponent::EnterCombatMode() {}

void UCombatComponent::ExitCombatMode() {}

float UCombatComponent::CalculateDamage(AActor *Target, UItemWeapon *Weapon)
{
    return 0.0f;
}

float UCombatComponent::CalculatePower(AActor *Target, UItemWeapon *Weapon)
{
    return 0.0f;
}

bool UCombatComponent::HasAlreadyHitTarget(AActor *Target)
{
    return false;
}

UCombatStance *UCombatComponent::GetCurrentCombatStance()
{
    check(CurrentCombatStance != nullptr && "Combat component: no combat stance");
    return CurrentCombatStance;
}

void UCombatComponent::On_StanceChanged()
{
    //todo получить актуальный модификатор скоростей
    if (IsWeaponDrawn)    // заменить на апдейт и поставить флаг
        Character->GetMovementComponent()->CalculateMaxCombatSpeeds(1);
}

void UCombatComponent::On_Weapon_OnOff()
{
    //todo получить актуальный модификатор скоростей
    Character->GetMovementComponent()->CalculateMaxSpeeds(1);
}

void UCombatComponent::On_Blocking()
{
    //todo получить актуальный модификатор скоростей
    Character->GetMovementComponent()->CalculateMaxBlockingSpeed(1);
}

void UCombatComponent::On_Casting()
{
    //todo получить актуальный модификатор скоростей
    Character->GetMovementComponent()->CalculateMaxCastingSpeed(1);
}
