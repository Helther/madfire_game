// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterBase.h"
#include "BasePlayerController.h"



// Sets default values

ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	LevelComponent = CreateDefaultSubobject<ULevelComponent>("LevelComponent");
	CombatComponent = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
    CurrentCCState = ECCState::NONE;
}
/*
ACharacterBase::ACharacterBase(const FObjectInitializer &ObjectInitilizer) : 
	Super(ObjectInitilizer.SetDefaultSubobjectClass<ULevelComponent>("LevelComponent"))
{

}*/

UBaseMovementComponent *ACharacterBase::GetMovementComponent() const
{
	return Cast<UBaseMovementComponent>(ACharacter::GetMovementComponent());
}

UCombatComponent * ACharacterBase::GetCombatComponent() const
{
	check(CombatComponent != nullptr && "No CombatComponent")

	return CombatComponent;

}

float ACharacterBase::TakeDamage(float DamageAmount, const struct FDamageEvent &DamageEvent, class AController *EventInstigator, AActor *DamageCauser)
{
	auto DamageType = DamageEvent.DamageTypeClass.GetDefaultObject();

	auto PhysD = Cast<UPhysicDamage>(DamageType);
	auto MagicD = Cast<UMagicDamage>(DamageType);

	if (PhysD != nullptr)
	{
		auto Type = PhysD->Type;
		auto Force = PhysD->Force;
		DamageAmount -= CurrentDamageAbsorb.DamageMap[(int32)Type];
		DamageAmount = DamageAmount * (CurrentDamageResistance.DamageMap[(int32)Type] - 100) / 100;
		
		CurrentHealth += DamageAmount;

		if (CurrentHealth > MaxHealth)
			CurrentHealth = MaxHealth;
		//TODO: Death, Force, DamageItem
		return DamageAmount;
	}
	else if (MagicD != nullptr)
	{
		auto Type = MagicD->Type;
		auto MagicSchool = MagicD->MagicSchool;
		DamageAmount -= CurrentDamageAbsorb.DamageMap[(int32)Type];
		DamageAmount = DamageAmount * (CurrentDamageResistance.DamageMap[(int32)Type] - 100) / 100;

		//TODO: Magic Armor
		CurrentHealth += DamageAmount;

		if (CurrentHealth > MaxHealth)
			CurrentHealth = MaxHealth;

		return DamageAmount;
	}
	else if (DamageType) //if damage nor Physical or Magical, then itВґs raw Damage
	{
		CurrentHealth -= DamageAmount;
		return DamageAmount;
	}

	return 0;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();	
	//update();
}

void ACharacterBase::OnUpdate()
{
	this->update();
}

void ACharacterBase::update()
{
	LevelComponent->update();

	float hk = CurrentHealth / MaxHealth; //РЎРѕРѕС‚РЅРѕС€РµРЅРёРµ Р—РґРѕСЂРѕРІСЊСЏ
	float sk = CurrentStamina / MaxStamina; //РЎРѕРѕС‚РЅРѕС€РµРЅРёРµ РЎС‚Р°РјРёРЅС‹

	CurrentAttributes = BaseAttributes + LevelComponent->LevelAttributes;
	CurrentSkills = BaseSkills + LevelComponent->LevelSkills;
	CurrentDamageResistance = BaseDamageResistance; // + Other Damage Resistance
	CurrentDamageAbsorb = BaseDamageAbsorb; // + Other Damage Absorb
	CurrentDamageBonus = BaseDamageBonus; // + Other Damage Bonus

	auto ConMod = UAttributesHelper::GetModifier(CurrentAttributes, EAttributes::Con);
	if (ConMod < 0) 
		ConMod = 0;

	MaxHealth = LevelComponent->LevelHealthBonus + (ConMod * LevelComponent->GetLevelNum());
	CurrentHealth = MaxHealth * hk;

	auto StrMod = UAttributesHelper::GetModifier(CurrentAttributes, EAttributes::Con);
	if (StrMod < 0)
		StrMod = 0;
	MaxStamina = 100 + (LevelComponent->GetLevelNum() * ConMod) + (LevelComponent->GetLevelNum() * StrMod);
	CurrentStamina = MaxStamina * sk;

	//Max Speed update
	CalculateMaxSpeed(1); //TODO

	//other calculation
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACharacterBase::OnWalkingOffLedge_Implementation(const FVector &PreviousFloorImpactNormal, const FVector &PreviousFloorContactNormal, const FVector &PreviousLocation, float TimeDelta)
{
    UE_LOG(LogTemp, Warning, TEXT("OnWalkingOffLedge"));
    ABasePlayerController *PlayerController = Cast<ABasePlayerController>(GetController());
    if(PlayerController->IsActionKeyPressed(FInputKeyNames::ClimbDownTest))
    {
        UE_LOG(LogTemp, Warning, TEXT("climb down"));
        GetMovementComponent()->ClimbDownWall();
    }
}

void ACharacterBase::CalculateMaxSpeed(float SpeedModifier)
{
	GetMovementComponent()->MaxWalkSpeed = GetMovementComponent()->MaxWalkSpeedDefault;
	GetMovementComponent()->MaxWalkSpeedCrouched = GetMovementComponent()->MaxWalkSpeedCrouchedDefault;
	GetMovementComponent()->MaxRunSpeed = GetMovementComponent()->MaxRunSpeedDefault * SpeedModifier;
	GetMovementComponent()->MaxRunSpeedCrouched = GetMovementComponent()->MaxRunSpeedCrouchedDefault * SpeedModifier;
	GetMovementComponent()->MaxSprintSpeed = GetMovementComponent()->MaxSprintSpeedDefault * SpeedModifier;
	GetMovementComponent()->MaxCustomMovementSpeed = GetMovementComponent()->MaxWallClimbSpeedDefault * SpeedModifier;
	GetMovementComponent()->MaxSwimSpeed = GetMovementComponent()->MaxSwimSpeedDefault * SpeedModifier;
	GetMovementComponent()->MaxFlySpeed = GetMovementComponent()->MaxFlySpeedDefault * SpeedModifier;

}

