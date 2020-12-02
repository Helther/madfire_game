#include "ConsumableAttributes.h"
#include <Character/AttributesComponent.h>

UConsumableAttribute::UConsumableAttribute()
{
	//SetTickableWhenPaused(false);
	//SetComponentTickEnabled(false);
}

bool UConsumableAttribute::Consume(float amount)
{
	const float result = CurrentVal + amount;
	if(result <= 0)
	{
		CurrentVal = 0;
		NoConsumeLeft();
		return false;
	}
	if(result >= CurrentMaxVal)
		CurrentVal = CurrentMaxVal;
	else
		CurrentVal = result;
	return true;
}


URegenConsumable::URegenConsumable() : Super()
{
	CurrentRegenRate = BaseRegenRate;
	RegenInterval = 1; // temp value, seconds between ticks
}

void URegenConsumable::BlockRegen()
{
	++RegenBlocker;
	UE_LOG(LogTemp, Warning, TEXT("block %d"), RegenBlocker);
	if(!IsRegenBlocked())
		GetWorld()->GetTimerManager().ClearTimer(RegenTickTimer);
}

void URegenConsumable::UnblockRegen()
{
	if(RegenBlocker > 0)
	{
		--RegenBlocker;
	}
	UE_LOG(LogTemp, Warning, TEXT("unblock %d"), RegenBlocker);
	if(!IsRegenBlocked())
		EnableRegen();
}

void URegenConsumable::EnableRegen()
{ 
	// UGameplayStatics::IsGamePaused()
		UE_LOG(LogTemp, Warning, TEXT("enable regen"));
	if(	!IsRegenBlocked() && GetValue() < GetCurrentMaxValue())
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

bool URegenConsumable::Consume(float amount)
{
	bool ret = Super::Consume(amount);
	if(!IsRegenBlocked())// todo check w/o it
		EnableRegen();
	return ret;
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
	if(!IsRegenBlocked())
			EnableRegen();
}
