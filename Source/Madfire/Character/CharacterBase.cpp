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
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>("EquipmentComponent");
	CurrentCCState = ECCState::NoCC;
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

UEquipmentComponent* ACharacterBase::GetEquipmentComponent() const
{
	check(EquipmentComponent != nullptr && "No EquipmentComponent")
	return EquipmentComponent;
}

float ACharacterBase::TakeDamage(float DamageAmount,
								const struct FDamageEvent &DamageEvent,
								class AController *EventInstigator,
								AActor *DamageCauser)
{
	const auto DamageType = DamageEvent.DamageTypeClass.GetDefaultObject();

	const auto PhysD = Cast<UPhysicDamage>(DamageType);
	const auto MagicD = Cast<UMagicDamage>(DamageType);

	if (PhysD != nullptr)
	{
		auto Type = PhysD->Type;
		auto Force = PhysD->Force;
		DamageAmount -= GetAttributesComponent()->CurrentDamageAbsorb.DamageMap[(int32)Type];
		DamageAmount = DamageAmount * 
			(GetAttributesComponent()->CurrentDamageResistance.DamageMap[(int32)Type] - 100) / 100;
		GetAttributesComponent()->Health->Consume(DamageAmount);// -> bool
		//TODO: Death, Force, DamageItem
		return DamageAmount;
	}
	else if (MagicD != nullptr)
	{// тоже что и физический + проверка на пробитие магического щита если такой имеется
		auto Type = MagicD->Type;
		auto MagicSchool = MagicD->MagicSchool;
		DamageAmount -= GetAttributesComponent()->CurrentDamageAbsorb.DamageMap[(int32)Type];
		DamageAmount = DamageAmount * 
			(GetAttributesComponent()->CurrentDamageResistance.DamageMap[(int32)Type] - 100) / 100;
		if(DamageAmount < 0)
		{
			if(GetAttributesComponent()->Health->Consume(DamageAmount))
				return DamageAmount = 0;
			else
				return DamageAmount = DamageAmount - GetAttributesComponent()->MagicShield->GetValue();
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
	if(GetLevelComponent()->IsUpdateNeeded())
		GetLevelComponent()->Update();
	if(GetAttributesComponent()->IsUpdateNeeded())
		GetAttributesComponent()->Update();
	if(GetEquipmentComponent()->IsUpdateNeeded())
		GetEquipmentComponent()->Update();
	if(GetMovementComponent()->IsUpdateNeeded())		
		GetMovementComponent()->Update();

	//other calculation
}

void ACharacterBase::DoLevelUp(const FLevelInfo& levelInfo) // todo replace in BP
{
	GetLevelComponent()->LevelUp(levelInfo);
	GetLevelComponent()->SetUpdate();
	GetAttributesComponent()->SetUpdate();
	bool dexChanged = UAttributesHelper::GetAttribute(levelInfo.Attributes, EAttributes::Dex) != 0;
	bool strChanged = UAttributesHelper::GetAttribute(levelInfo.Attributes, EAttributes::Str) != 0;
	bool acrobaticsChanged = USkillHelper::GetValue(levelInfo.SkillList, ESkill::Tumble) != 0;
	if(strChanged)
		GetEquipmentComponent()->SetUpdate();
	if(dexChanged || strChanged || acrobaticsChanged)
		GetMovementComponent()->SetUpdate();
	// может что то еще
	Update();
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
