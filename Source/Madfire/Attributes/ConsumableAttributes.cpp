#include "ConsumableAttributes.h"
#include <Character/AttributesComponent.h>

UConsumableAttribute::UConsumableAttribute()
{
	//SetTickableWhenPaused(false);
	//SetComponentTickEnabled(false);
}

void UConsumableAttribute::Consume(float amount)
{
	const float result = CurrentVal + amount;
	if(result <= 0)
	{
		CurrentVal = 0;
		NoConsumeLeft();
	}
	else if(result >= CurrentMaxVal)
		CurrentVal = CurrentMaxVal;
	else
		CurrentVal = result;
}


URegenConsumable::URegenConsumable() : Super()
{
	RegenInterval = 1; // temp value, seconds between ticks
}

void URegenConsumable::BlockRegen()
{
	++RegenBlocker;
	if(!IsRegenBlocked())
		GetWorld()->GetTimerManager().ClearTimer(RegenTickTimer);
}

void URegenConsumable::UnblockRegen()
{
	if(RegenBlocker > 0) --RegenBlocker;
	if(!IsRegenBlocked())
		EnableRegen();
}

void URegenConsumable::EnableRegen()
{ 
	// UGameplayStatics::IsGamePaused()
	if(GetValue() < GetCurrentMaxValue())
	{
		SetValue(GetValue() + GetCurrentRegenRate());
		if(GetValue() < GetCurrentMaxValue())
		{
			if(!GetWorld()->GetTimerManager().IsTimerActive(RegenTickTimer))
				GetWorld()->GetTimerManager().SetTimer(RegenTickTimer, this, &URegenConsumable::OnRegenTick, 
											RegenInterval, true);
			return;
		}
		SetValue(GetCurrentMaxValue());
	}
	if(GetWorld()->GetTimerManager().IsTimerActive(RegenTickTimer))
	GetWorld()->GetTimerManager().ClearTimer(RegenTickTimer);
}
void URegenConsumable::NoConsumeLeft()
{
	NoConsumeBlock = true;
	if(GetWorld()->GetTimerManager().IsTimerActive(RegenTickTimer))
		GetWorld()->GetTimerManager().ClearTimer(RegenTickTimer);
	GetWorld()->GetTimerManager().SetTimer(NoConsumeTimer, this, &URegenConsumable::OnNoConsumeTimeFinish, 
											NoConsumeTime, true);
}

void URegenConsumable::Consume(float amount)
{
	Super::Consume(amount);
	if(!IsRegenBlocked())
		EnableRegen();
}

void URegenConsumable::OnNoConsumeTimeFinish()
{
	NoConsumeBlock = false;
	if(!IsRegenBlocked())
		EnableRegen();
}

UWorld* URegenConsumable::GetWorld() const
{
	UAttributesComponent* parent = Cast<UAttributesComponent>(GetOuter());
	check(parent != nullptr && "Consume atrr: no parent obj")
	return parent->GetWorld();
}

void URegenConsumable::OnRegenTick()
{
	EnableRegen();
	UE_LOG(LogTemp,Warning, TEXT("Health %f"), GetValue())
	UE_LOG(LogTemp,Warning, TEXT("RegenRate %f"), GetCurrentRegenRate())
}


void UHealthContainer::NoConsumeLeft()
{
	Super::NoConsumeLeft();
	// todo что то при смерти
}


void UStaminaContainer::NoConsumeLeft()
{
	Super::NoConsumeLeft();
}
