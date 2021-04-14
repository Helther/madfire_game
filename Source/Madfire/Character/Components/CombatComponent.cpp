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
        CalculateMaxCombatSpeeds();
}

void UCombatComponent::On_Weapon_OnOff()
{
    if (IsWeaponDrawn)
    {
        Character->GetMovementComponent()->maxSpeedBlock = true;
        CalculateMaxCombatSpeeds();
    }
    else
        Character->GetMovementComponent()->maxSpeedBlock = false;
}

void UCombatComponent::On_Blocking()
{
    //todo учесть выход из состояния
    CalculateMaxBlockingSpeed(Character->GetMovementComponent()->GetSpeedModifier());
}

void UCombatComponent::On_Casting()
{
    //todo учесть выход из состояния
    CalculateMaxCastingSpeed(Character->GetMovementComponent()->GetSpeedModifier());
}


void UCombatComponent::CalculateMaxCombatSpeeds()
{
    float speedMod = Character->GetMovementComponent()->GetSpeedModifier();
    Character->GetMovementComponent()->MaxRunSpeed =
        GetCurrentCombatStance()->MaxCombatRunSpeed * speedMod;
}

void UCombatComponent::CalculateMaxBlockingSpeed(float SpeedModifier)
{
    Character->GetMovementComponent()->MaxRunSpeed =
      Character->CombatComponent->GetCurrentCombatStance()->MaxBlockSpeed * SpeedModifier;
}

void UCombatComponent::CalculateMaxCastingSpeed(float SpeedModifier)
{
    Character->GetMovementComponent()->MaxRunSpeed =
      Character->CombatComponent->GetCurrentCombatStance()->MaxCastingSpeed
      * SpeedModifier;
}
