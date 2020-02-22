
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Attributes/Attributes.h"
#include "../Attributes/Skills.h"
#include "../Attributes/DamageMap.h"
#include "../Attributes/Damage.h"
#include "LevelList.h"
#include "PlayerMovementComponent.h"
#include "CombatComponent.h"
#include "CharacterBase.generated.h"

UENUM(BluePrintType)
enum class ECCState : uint8
{
	NONE,
	Stun,
	KnockDown,
	LossOfBalance,
	Stasis,
	Sleep,
	MAX
}; // состояния контроля персонажа

UCLASS()
class MADFIRE_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase(const FObjectInitializer& ObjectInitializer);
	//ACharacterBase(const FObjectInitializer &ObjectInitilizer);

	float TakeDamage(float DamageAmount, const struct FDamageEvent &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;

	virtual void update();

	UFUNCTION(BlueprintCallable, Category = "Character Base")
		void OnUpdate(); //!< Blueprint interface  for Blueprints

	class UBaseMovementComponent *GetMovementComponent() const override;

    UCombatComponent* GetCombatComponent() const;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input 
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	//Base stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float MaxHealth = 100;		//!< Максимальное здоровье (базовое, без учётов бонусов)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float MaxStamina = 100;		//!< Максимальная стамина (базовая, без учётов бонусов)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		FAttributes BaseAttributes; //!< Базовые аттрибуты (сила, ловкость и т.д.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		FSkillList  BaseSkills;     //!< Базовые скилы (дипломатия, кузнечество и т.д.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		FDamageResistance BaseDamageResistance;  //!< Базовое снижение урона (в процентах)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		FDamageAbsorb BaseDamageAbsorb;       //!< Базовое снижение урона (на число (поглощение))
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		FDamageAbsorb BaseDamageBonus;       //!< Базовая прибавка к урону (на число или проценты?)

	//! Current stats - текущие значения. Это значения полученные при пересчёте, кроме здоровья и стамины
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float CurrentHealth = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float CurrentStamina = 100;
	UPROPERTY(BlueprintReadWrite, Category = "Character Stats")
		float CurrentHealthRegen;
	UPROPERTY(BlueprintReadWrite, Category = "Character Stats")
		float CurrentStaminaRegen;
	UPROPERTY(BlueprintReadWrite, Category = "Character Stats")
		FAttributes CurrentAttributes;
	UPROPERTY(BlueprintReadWrite, Category = "Character Stats")
		FSkillList CurrentSkills;
	UPROPERTY(BlueprintReadWrite, Category = "Character Stats")
		FDamageResistance CurrentDamageResistance;
	UPROPERTY(BlueprintReadWrite, Category = "Character Stats")
		FDamageAbsorb CurrentDamageAbsorb;
	UPROPERTY(BlueprintReadWrite, Category = "Character Stats")
		FDamageAbsorb CurrentDamageBonus;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float AttackSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float CastSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		ECCState CurrentCCState;

    virtual void OnWalkingOffLedge_Implementation( const FVector & PreviousFloorImpactNormal,
                                                   const FVector & PreviousFloorContactNormal,
                                                   const FVector & PreviousLocation,
                                                   float TimeDelta) override;
protected:
	UPROPERTY(BlueprintReadWrite, Category = "Character Components")
		ULevelComponent *LevelComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Character Components")
		UCombatComponent *CombatComponent;

	void CalculateMaxSpeed(float SpeedModifier); // для обновления максимальныз скоростей


};
