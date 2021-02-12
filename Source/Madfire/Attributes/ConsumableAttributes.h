#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

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

    // возвращает фолс если не хватило ресурса
    UFUNCTION(BlueprintCallable, Category = "Character Stats")
    virtual bool Consume(float amount);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseMaxVal = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentVal = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentMaxVal = 0;

protected:
    virtual void NoConsumeLeft(){};
};

// Зарефакторить реген блок с исопльзованием счетчика ссылок
// "интерфейс для регенируемых поглощаемых аттрибутов"
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew)
class MADFIRE_API URegenConsumable : public UConsumableAttribute
{
    GENERATED_BODY()

public:
    URegenConsumable();
    bool IsRegenBlocked() const { return (RegenBlocker > 0 || NoConsumeBlock); }
    void BlockRegen();
    void UnblockRegen();
    void ResetRegenBlock()
    {
        if (RegenBlocker != 0)
            RegenBlocker = 0;
    }
    void SetCurrentRegenRate(float NewVal)
    {
        if (NewVal < 0)
            NewVal = 0;
        CurrentRegenRate = NewVal;
    }
    float GetCurrentRegenRate() const { return CurrentRegenRate; }
    float GetBaseRegenRate() const { return BaseRegenRate; }
    bool Consume(float amount) override;
    void OnRegenTick();

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

protected:
    class UWorld *GetWorld() const override;
    void OnNoConsumeTimeFinish();
    void NoConsumeLeft() override;

    // увеличивается при каждом вызове BlockRegen на 1
    int RegenBlocker = 0;
    // если уменьшается до нуля устанавливается то тру
    bool NoConsumeBlock    = false;
    float CurrentRegenRate = 0;
};
