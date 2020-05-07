// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Attributes/Attributes.h"
#include "../Attributes/Skills.h"
#include "../Attributes/DamageMap.h"
#include "../Attributes/Damage.h"
#include "AttributesComponent.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MADFIRE_API UAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttributesComponent();
	//todo добавить текущие максимальные значения хп и стамины как минимум
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

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
