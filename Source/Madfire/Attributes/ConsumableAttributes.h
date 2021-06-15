#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Components/ActorComponent.h"
#include "ConsumableAttributes.generated.h"

// ниже скрывается иерархия классов, ответственная за контроль над
// поглощаемыми ресурсами, например, маг. щит, некоторые из них
// регенирируют с некоей скоростью (хп, стамина)




USTRUCT(BlueprintType)
struct FAttributeData
{
    GENERATED_BODY()

    FAttributeData() :
      BaseValue(1.f),
      BonusValue(0.f),
      CurrentValue(1.f)
    {}

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Attribute")
    float BaseValue;

    UPROPERTY(BlueprintReadWrite, Category = "Attribute")
    float BonusValue;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "Attribute")
    float CurrentValue; //< Edit through the Consume(float)

    float GetMaxValue() const {return BaseValue + BonusValue;}

    bool Consume(float Amount)
    {
        const float result = CurrentValue + Amount;
        if(result <= 0)
        {
            CurrentValue = 0;
            return false;
        }
        if(result >= GetMaxValue())
            CurrentValue = GetMaxValue();
        else
            CurrentValue = result;
        return true;
    }
};


UCLASS(BlueprintType, EditInlineNew)
class UDynamicAttribute : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Attribute")
    float BaseValue = 1.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attribute")
    float BonusValue = 10.f;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "Attribute")
    float CurrentValue = 1.f; //< Edit through the Consume(float)

    FTimerHandle RegenTimer;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Attribute")
    float RegenValue = 0;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Attribute")
    float RegenCoef = 1.f;

    UPROPERTY(SaveGame, EditAnywhere, Category = "Attribute")
    float NoConsumeTime = 10;

    UPROPERTY(SaveGame, EditAnywhere, Category = "Attribute")
    float RegenRate = 50;

    UFUNCTION(BlueprintPure,  Category = "Attribute")
    float GetMaxValue() const {return BaseValue + BonusValue;}

    int32 RegenBlocker = 0;

    void ClearRegenTimer()
    {
        GetTimerManager().ClearTimer(RegenTimer);
    }

    void StartRegenTimer()
    {
        GetTimerManager().ClearTimer(RegenTimer);
        GetTimerManager().SetTimer(RegenTimer, this, &UDynamicAttribute::RegenTick, RegenRate, true, NoConsumeTime);
    }

    void RegenTick()
    {
        if(!CanRegen())
        {
            GetTimerManager().PauseTimer(RegenTimer);
            return;
        }

        if(CurrentValue < GetMaxValue())
        {
            CurrentValue += GetRegenValue();
            if(CurrentValue >= GetMaxValue())
            {
                CurrentValue = GetMaxValue();
                GetTimerManager().PauseTimer(RegenTimer);
            }
        }
    }

    UFUNCTION(BlueprintPure)
    bool CanRegen() const {return RegenBlocker == 0;}

    UFUNCTION(BlueprintCallable)
    void BlockRegen()
    {
        ++RegenBlocker;
        if(!GetTimerManager().IsTimerPaused(RegenTimer))
            GetTimerManager().PauseTimer(RegenTimer);
    }

    float GetRegenValue() const
    {
        return RegenCoef * RegenValue;
    }

    UFUNCTION(BlueprintCallable)
    void UnblockRegen(bool Absolute = false)
    {
        if(Absolute || RegenBlocker < 0)
            RegenBlocker = 0;

        if(RegenBlocker)
            --RegenBlocker;

        if(CanRegen() && GetTimerManager().IsTimerPaused(RegenTimer))
            GetTimerManager().UnPauseTimer(RegenTimer);
    }

    UFUNCTION(BlueprintCallable)
    bool Consume(float Amount)
    {
        const float result = CurrentValue + Amount;
        if(result <= 0)
        {
            CurrentValue = 0;
            StartRegenTimer();
            return false;
        }
        if(result >= GetMaxValue())
            CurrentValue = GetMaxValue();
        else
            CurrentValue = result;
        return true;
    }



    void SetWorld(UWorld *InWorld) { World = InWorld;}
    class UWorld *GetWorld() const override { return World; }

private:
    FTimerManager &GetTimerManager() const;
    UWorld *World;
};






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
    virtual void NoConsumeLeft() {};
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
