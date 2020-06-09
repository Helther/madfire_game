// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterBase.h"



ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AttributesComponent = CreateDefaultSubobject<UAttributesComponent>("AttributesComponent");
	LevelComponent = CreateDefaultSubobject<ULevelComponent>("LevelComponent");
	CombatComponent = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
    CurrentCCState = ECCState::NONE;
}

UBaseMovementComponent* ACharacterBase::GetMovementComponent() const
{
	return Cast<UBaseMovementComponent>(ACharacter::GetMovementComponent());
}

UCombatComponent* ACharacterBase::GetCombatComponent() const
{
	check(CombatComponent != nullptr && "No CombatComponent")
	return CombatComponent;
}

UAttributesComponent* ACharacterBase::GetAttributesComponent() const
{	
	check(AttributesComponent != nullptr && "No AttrComponent")
	return AttributesComponent;
}

ULevelComponent* ACharacterBase::GetLevelComponent() const
{	
	check(LevelComponent != nullptr && "No LevelComponent")
	return LevelComponent;
}

float ACharacterBase::TakeDamage(float DamageAmount,
								const struct FDamageEvent &DamageEvent,
								class AController *EventInstigator,
								AActor *DamageCauser)
{
	auto DamageType = DamageEvent.DamageTypeClass.GetDefaultObject();

	auto PhysD = Cast<UPhysicDamage>(DamageType);
	auto MagicD = Cast<UMagicDamage>(DamageType);

	if (PhysD != nullptr)
	{
		auto Type = PhysD->Type;
		auto Force = PhysD->Force;
		DamageAmount -= GetAttributesComponent()->CurrentDamageAbsorb.DamageMap[(int32)Type];
		DamageAmount = DamageAmount * (GetAttributesComponent()->CurrentDamageResistance.DamageMap[(int32)Type] - 100) / 100;
		
		GetAttributesComponent()->CurrentHealth += DamageAmount;

		if (GetAttributesComponent()->CurrentHealth > GetAttributesComponent()->MaxHealth)
			GetAttributesComponent()->CurrentHealth = GetAttributesComponent()->MaxHealth;
		//TODO: Death, Force, DamageItem
		return DamageAmount;
	}
	else if (MagicD != nullptr)
	{
		auto Type = MagicD->Type;
		auto MagicSchool = MagicD->MagicSchool;
		DamageAmount -= GetAttributesComponent()->CurrentDamageAbsorb.DamageMap[(int32)Type];
		DamageAmount = DamageAmount * (GetAttributesComponent()->CurrentDamageResistance.DamageMap[(int32)Type] - 100) / 100;

		//TODO: Magic Armor
		GetAttributesComponent()->CurrentHealth += DamageAmount;

		if (GetAttributesComponent()->CurrentHealth > GetAttributesComponent()->MaxHealth)
			GetAttributesComponent()->CurrentHealth = GetAttributesComponent()->MaxHealth;

		return DamageAmount;
	}
	else if (DamageType) //if damage nor Physical or Magical, then itВґs raw Damage
	{
		GetAttributesComponent()->CurrentHealth -= DamageAmount;
		return DamageAmount;
	}

	return 0;
}
void ACharacterBase::OnJumped_Implementation()
{
	GetMovementComponent()->bCanHangAgain = false;
	GetMovementComponent()->bWasJustJumping = true;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();	
}

void ACharacterBase::OnUpdate()
{
	this->Update();
}

void ACharacterBase::Update()
{
	GetLevelComponent()->Update();
	float hk = GetAttributesComponent()->CurrentHealth / GetAttributesComponent()->MaxHealth; 
	float sk = GetAttributesComponent()->CurrentStamina / GetAttributesComponent()->MaxStamina; 

	GetAttributesComponent()->CurrentAttributes = GetAttributesComponent()->BaseAttributes + 
	GetLevelComponent()->LevelAttributes;
	GetAttributesComponent()->CurrentSkills = GetAttributesComponent()->BaseSkills + 
	GetLevelComponent()->LevelSkills;
	GetAttributesComponent()->CurrentDamageResistance = GetAttributesComponent()->BaseDamageResistance; // + Other Damage Resistance
	GetAttributesComponent()->CurrentDamageAbsorb = GetAttributesComponent()->BaseDamageAbsorb; // + Other Damage Absorb
	GetAttributesComponent()->CurrentDamageBonus = GetAttributesComponent()->BaseDamageBonus; // + Other Damage Bonus

	auto ConMod = UAttributesHelper::GetModifier(GetAttributesComponent()->CurrentAttributes, EAttributes::Con);
	if (ConMod < 0) 
		ConMod = 0;

	GetAttributesComponent()->MaxHealth = GetLevelComponent()->LevelHealthBonus + (ConMod * GetLevelComponent()->GetLevelNum());
	GetAttributesComponent()->CurrentHealth = GetAttributesComponent()->MaxHealth * hk;

	auto StrMod = UAttributesHelper::GetModifier(GetAttributesComponent()->CurrentAttributes, EAttributes::Str);
	if (StrMod < 0)
		StrMod = 0;
	GetAttributesComponent()->MaxStamina = 100 + (GetLevelComponent()->GetLevelNum() * ConMod) + (GetLevelComponent()->GetLevelNum() * StrMod);
	GetAttributesComponent()->CurrentStamina = GetAttributesComponent()->MaxStamina * sk;

	GetMovementComponent()->Update();
	
	//other calculation
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}