// Fill out your copyright notice in the Description page of Project Settings.

#include "AttributesComponent.h"
#include "CharacterBase.h"

// Sets default values for this component's properties
UAttributesComponent::UAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	/* Some shitty ways to init uObjects
	//Health = CreateDefaultSubobject<UHealthContainer>("Health");
	//Stamina = CreateDefaultSubobject<UStaminaContainer>("Stamina");
	//MagicShield = CreateDefaultSubobject<UMagicShieldContainer>("MShield");
		HealthClass = UHealthContainer::StaticClass();
	StaminaClass = UStaminaContainer::StaticClass();
	MagicShieldClass = UMagicShieldContainer::StaticClass();
	check(HealthClass->IsValidLowLevelFast());
	check(StaminaClass->IsValidLowLevelFast());
	check(MagicShieldClass->IsValidLowLevelFast());
	Health = NewObject<UHealthContainer>(this, HealthClass->GetFName(), RF_NoFlags, HealthClass->GetDefaultObject());
	Stamina = NewObject<UStaminaContainer>(this, StaminaClass->GetFName(), RF_NoFlags, StaminaClass->GetDefaultObject());
	MagicShield = NewObject<UMagicShieldContainer>(this, MagicShieldClass->GetFName(), RF_NoFlags, MagicShieldClass->GetDefaultObject());
	*/

	// ...
}

void UAttributesComponent::Update()
{
	float hk = Health->GetValue() / Health->GetCurrentMaxValue(); 
	float sk = Stamina->GetValue() / Stamina->GetCurrentMaxValue(); 

	CurrentAttributes = BaseAttributes + Character->GetLevelComponent()->LevelAttributes; // + equipment
	CurrentSkills = BaseSkills + Character->GetLevelComponent()->LevelSkills; // + equipment
	CurrentDamageResistance = BaseDamageResistance; // + Other Damage Resistance(equipment)
	CurrentDamageAbsorb = BaseDamageAbsorb; // + Other Damage Absorb(equipment)
	CurrentDamageBonus = BaseDamageBonus; // + Other Damage Bonus(equipment)

	auto ConMod = UAttributesHelper::GetModifier(CurrentAttributes, EAttributes::Con);
	if(ConMod < 0) 
		ConMod = 0;	// todo нужно ли?

	Health->SetCurrentMaxVal(Health->GetBaseMaxValue() + 
													Character->GetLevelComponent()->LevelHealthBonus + (ConMod * 
													Character->GetLevelComponent()->GetLevelNum()));
	Health->SetValue(Health->GetCurrentMaxValue() * hk);

	auto StrMod = UAttributesHelper::GetModifier(CurrentAttributes, EAttributes::Str);
	if(StrMod < 0)
		StrMod = 0; // todo нужно ли?
	Stamina->SetCurrentMaxVal(Stamina->GetBaseMaxValue() + 
													(Character->GetLevelComponent()->GetLevelNum() * ConMod) + 
													(Character->GetLevelComponent()->GetLevelNum() * StrMod));
	Stamina->SetValue(Stamina->GetCurrentMaxValue() * sk);

	Health->SetCurrentRegenRate(Health->GetBaseRegenRate() + ConMod);
	Stamina->SetCurrentRegenRate(Stamina->GetBaseRegenRate() + ConMod);

	UnSetUpdate();
}

// Called when the game starts
void UAttributesComponent::BeginPlay()
{
	Super::BeginPlay();
	Character = Cast<ACharacterBase>(GetOwner());
  check(Character != nullptr && "UBaseMovementComponent error: No pawn")

	if(!Health->IsRegenBlocked())
		Health->EnableRegen();
	if(!Stamina->IsRegenBlocked())
		Stamina->EnableRegen();
	// ...
}

// Called every frame
void UAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
