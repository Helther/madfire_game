// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributesComponent.h"


// Sets default values for this component's properties
UAttributesComponent::UAttributesComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	Health = NewObject<UHealthContainer>();
	Stamina = NewObject<UStaminaContainer>();
	MagicShield = NewObject<UMagicShieldContainer>();

	Health->SetBaseMaxValue(100);
	Health->SetCurrentMaxVal(100);
	Health->SetValue(100);
	Stamina->SetBaseMaxValue(100);
	Stamina->SetCurrentMaxVal(100);
	Stamina->SetValue(100);
	// ...
}

// Called when the game starts
void UAttributesComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UConsumableAttributr::Consume(float amount)
{
	const auto result = CurrentVal + amount;
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


void UHealthContainer::NoConsumeLeft()
{
	// todo что то при смерти
}

void UStaminaContainer::NoConsumeLeft()
{
	RegenBlockSemaphore = -1;
	GetWorld()->GetTimerManager().SetTimer(NoConsumTimer, this, &UStaminaContainer::OnNoConsumeTimeFinish, 
											NoConsumTime, true);
}

void UMagicShieldContainer::Consume(float amount)
{
	if(amount < 0)
	{
		CurrentVal += amount;
		if(CurrentVal < 0)
		{
			CurrentVal = 0;
			NoConsumeLeft();
		}
	}
}