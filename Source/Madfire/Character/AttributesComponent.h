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
		float AttackSpeed;	//!< в процентах
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float CastSpeed;	//!< в процентах

	UPROPERTY(BlueprintReadWrite, Category = "Character Stats")
		UHealthContainer *Health;
	UPROPERTY(BlueprintReadWrite, Category = "Character Stats")
		UStaminaContainer *Stamina;
	UPROPERTY(BlueprintReadWrite, Category = "Character Stats")
		UMagicShieldContainer *MagicShield;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};


// todo add descr
UCLASS(Abstract)
class UConsumableAttributr : public UObject
{
	GENERATED_BODY()
public:
	virtual void SetValue(float NewVal){ CurrentVal = NewVal; }
	virtual void SetBaseMaxValue(float NewVal){ BaseMaxVal = NewVal; }
	virtual void SetCurrentMaxVal(float NewVal){ CurrentMaxVal = NewVal; }
	virtual float GetValue() const { return CurrentVal; }
	virtual float GetBaseMaxValue() const { return BaseMaxVal; }
	virtual float GetCurrentMaxValue() const { return CurrentMaxVal; }
	virtual void Consume(float amount);
	
protected:
	virtual void NoConsumeLeft(){};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float BaseMaxVal = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float CurrentVal = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float CurrentMaxVal = 0;				
};

// todo how to handle regen rate update
UCLASS(Abstract)
class URegenConsumabale : public UConsumableAttributr
{
	GENERATED_BODY()

public:
	bool IsRegenBlocked() const { return (RegenBlockSemaphore > 0 || 
							RegenBlockSemaphore == -1); }
	void BlockRegen(){ if(!(RegenBlockSemaphore < 0)) ++RegenBlockSemaphore; }
	void UnblockRegen(){ if(RegenBlockSemaphore > 0) --RegenBlockSemaphore; }
	void ResetRegenBlock(){ if(RegenBlockSemaphore != 0) RegenBlockSemaphore = 0; }
	void SetRegenRate(float NewVal){ RegenRate = NewVal; }
	float GetRegenRate() const { return RegenRate;}

protected:
	virtual void OnNoConsumeTimeFinish(){};

	// увеличивается при каждом вызове BlockRegen на 1
	// если уменьшается до нуля устанавливается = -1
	int RegenBlockSemaphore = 0;
	// время блокировки регена при опустошении 
	float NoConsumTime = 0;// todo добавить обработку таймера
	// таймер блокировки регена 
	FTimerHandle NoConsumTimer;
	// число на которое увеличивается параметр за период времени
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		float RegenRate = 0;
};

UCLASS(BlueprintType)
class UHealthContainer : public URegenConsumabale
{
	GENERATED_BODY()

public:
private:
	void NoConsumeLeft() override;
};

UCLASS(BlueprintType)
class UStaminaContainer : public URegenConsumabale
{
	GENERATED_BODY()

public:

private:
	void NoConsumeLeft() override;
	void OnNoConsumeTimeFinish() override{
		RegenBlockSemaphore = 0;
	}
};

UCLASS(BlueprintType)
class UMagicShieldContainer : public UConsumableAttributr
{
	GENERATED_BODY()

public:
	void Consume(float amount) override;

private:
};
