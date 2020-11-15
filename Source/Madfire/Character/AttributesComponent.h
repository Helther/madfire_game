// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <Attributes/ConsumableAttributes.h>
#include "../Attributes/Attributes.h"
#include "../Attributes/Skills.h"
#include "../Attributes/DamageMap.h"
#include "../Attributes/Damage.h"
#include "../Attributes/ConsumableAttributes.h"
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
// clang-format off
	//============= Base stats
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

	//============== Current stats - текущие значения. 
	// Это значения полученные при пересчёте, кроме здоровья и стамины
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
		float AttackSpeed;	/*!< в процентах*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float CastSpeed;	/*!< в процентах*/
	// объект для упраления значениями хп (текущее, макс, реген)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Character Stats")
		UHealthContainer *Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Character Stats")
		UStaminaContainer *Stamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Character Stats")
		UMagicShieldContainer *MagicShield;
		/*
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UHealthContainer> HealthClass;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UStaminaContainer> StaminaClass;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UMagicShieldContainer> MagicShieldClass;
	*/
// clang-format on
	virtual void Update();
	UFUNCTION(BlueprintCallable, Category = "Component Update")// todo 
		void SetUpdate() { IsDirty = true; }
	UFUNCTION(BlueprintCallable, Category = "Component Update")
		void UnSetUpdate() { IsDirty = false; }
	UFUNCTION(BlueprintCallable, Category = "Component Update")
		bool IsUpdateNeeded() { return IsDirty; }
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	private:
	    class ACharacterBase* Character;
			bool IsDirty = false;
};
