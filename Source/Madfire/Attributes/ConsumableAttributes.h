#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ConsumableAttributes.generated.h"

// ниже скрывается иерархия классов, ответственная за контроль над
// поглощаемыми ресурсами, например, маг. щит, некоторые из них 
// регенирируют с некоей скоростью (хп, стамина)

// "интерфейс" для поглощаемых аттрибутов
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew)
class MADFIRE_API UConsumableAttribute : public UObject
{
	GENERATED_BODY()

public:
	UConsumableAttribute();

	virtual void SetValue(float NewVal){ CurrentVal = NewVal; }
	virtual void SetBaseMaxValue(float NewVal){ BaseMaxVal = NewVal; }
	virtual void SetCurrentMaxVal(float NewVal){ CurrentMaxVal = NewVal; }
	virtual float GetValue() const { return CurrentVal; }
	virtual float GetBaseMaxValue() const { return BaseMaxVal; }
	virtual float GetCurrentMaxValue() const { return CurrentMaxVal; }
	//UFUNCTION(BlueprintCallable, Category = "Character Stats") 
	// возвращает фолс если не хватило ресурса
	virtual bool Consume(float amount);
	//virtual void Update(){}; // todo 

protected:
	virtual void NoConsumeLeft(){};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseMaxVal = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float CurrentVal = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float CurrentMaxVal = 0;				
};

// Зарефакторить реген блок с исопльзованием счетчика ссылок
// "интерфейс для регенируемых поглощаемых аттрибутов"
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew)
class MADFIRE_API URegenConsumable : public UConsumableAttribute
{
	GENERATED_BODY()

public:
	URegenConsumable();
	bool IsRegenBlocked() const { return (RegenBlocker > 0 || 
							NoConsumeBlock); }
	void BlockRegen();
	void UnblockRegen();
	void ResetRegenBlock() { if(RegenBlocker != 0) RegenBlocker = 0; }
	void SetCurrentRegenRate(float NewVal){ if(NewVal < 0) NewVal = 0; CurrentRegenRate = NewVal; }
	float GetCurrentRegenRate() const { return CurrentRegenRate; }
	float GetBaseRegenRate() const { return BaseRegenRate; }
	bool Consume(float amount) override;
	void EnableRegen();

protected:
	class UWorld* GetWorld() const override;
	virtual void OnNoConsumeTimeFinish();
	virtual void OnRegenTick();
	void NoConsumeLeft() override;

	// увеличивается при каждом вызове BlockRegen на 1
	int RegenBlocker = 0;
	// если уменьшается до нуля устанавливается то тру
	bool NoConsumeBlock = false;
	// время блокировки регена при опустошении 
	UPROPERTY(EditAnywhere)
		float NoConsumeTime = 3;
	// таймер блокировки регена 
	FTimerHandle NoConsumeTimer;
	// таймер апдейта регена
	FTimerHandle RegenTickTimer;
	// интервал в секундах между вызовами тика регена
	UPROPERTY(EditAnywhere)
		float RegenInterval = 0;
	// число на которое увеличивается параметр за период времени
	UPROPERTY(EditAnywhere)
		float BaseRegenRate = 5;
	float CurrentRegenRate = 0;
};
