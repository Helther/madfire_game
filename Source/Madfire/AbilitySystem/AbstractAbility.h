#pragma once

#include "Engine.h"
#include "CoreMinimal.h"
#include "AbilityComponent.h"
#include "AbilitySystemLog.h"
#include "GameFramework/Actor.h"
#include "AbstractRequirement.h"
#include "AbstractAbility.generated.h"



/*
* Тут я удалил всякие ненужные функции, оставил только интерфейс
* Интерфейс я тоже немного поменял - Убрал эти пустые реализации. Должно сработать
* Список Экшонов в скиле я заменил на список Компонентов (см. SkillComponent)
* Также там внизу есть мэнеджер, который может спавнить Скилы и компоненты к ним.
*/
UCLASS(BlueprintType, Blueprintable,  abstract)
class UAbstractAbility : public UObject
{
	GENERATED_BODY()

public:
	//Parameters (Внешние)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		bool isMovementDisabled = false; //still needs to implement

	//UPROPERTY(EditAnywhere, Instanced, Category = "Skill Config")
		//TArray<USkillAction *> ActionList;

	UPROPERTY(EditAnywhere, Category = "Skill Parameters")
		TArray<FAbilityComponentSpawnParameters> SkillComponents;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Skill Parameters")
		TArray<UAbilityRequirement*> Requirements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		float ChargeCDTime = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		int32 MaxCharges = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		float CastTime = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		float GlobalCoolDownTime = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		UTexture2D * SkillIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		UAnimSequence * CastAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		UAnimSequence * AttackLoop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		UAnimSequence * EndAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		FText SkillName;

public: //Interface
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spell System")
		void OnEventKeyPressed();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spell System")
		void OnEventKeyReleased();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spell System")
		void InterruptSkill();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spell System")
		void StopSkill();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Spell System")
		bool CheckRequirements();
	bool CheckRequirements_Implementation(void);



	//Internal parameters (Внутренние параметры)
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		ACharacter *Caster;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		bool onCooldown = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		bool isCasting = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		int32 CurrentCharges = 1;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		int CastState = 0;


protected:
	FTimerHandle CastTimer;

	inline void ConsumeRequirements(void)
	{
		for (auto &Requirment : Requirements)
		{
			Requirment->Consume(Caster);
		}
	}

	inline bool isValid(void)
	{
		check(Caster != nullptr && "Skill: caster not found");
		check(GlobalCoolDownTime > 0.0f && "Skill: cooldown time is not valid");
		check(CastTime > 0 && "Skill: charge cooldown time is not valid");
		check(MaxCharges > 0 && "Skill: max charge count is not valid");

	
		if (isCasting)
		{
			UE_LOG(AbilitySystemLog, Warning, TEXT("Skill: is casting now"));
			return false;
		}

		if (onCooldown)
		{
			UE_LOG(AbilitySystemLog, Warning, TEXT("Skill: is on cooldown now"));
			return false;
		}

		if (SkillComponents.Num() == 0)
		{
			UE_LOG(AbilitySystemLog, Error, TEXT("Skill: component list is empty"));
			return false;
		}

		return true;
	}
};


/*
 * Вот небольшой манагер для скилов
 */
UCLASS()
class UAbilityManager : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Create Skill From Class", CompactNodeTitle = "Create Skill", Keywords = "new create skill"), Category = "Spell System")
		static UAbstractAbility *CreateSkill(ACharacter *caster, TSubclassOf<UAbstractAbility> SkillClass);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Create Skill From Class", CompactNodeTitle = "Create Skill", Keywords = "new create skill"), Category = "Spell System")
		static AAbilityComponent *SpawnComponent(AActor *target, FAbilityComponentSpawnParameters SpawnParameters, int impower);
};
















