// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentComponent.h"
#include "CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "../TraceFunctional/TraceDefines.h"

// Sets default values for this component's properties
/*UEquipmentComponent::UEquipmentComponent() :
	Owner(nullptr)
{}*/

void UEquipmentComponent::Update()
{
	// todo подумать что еще можно заапдейтить
	// проверить что содержимое слота изменилось
	//CalculateWeight(); debug temp
	UnSetUpdate();
}

void UEquipmentComponent::OnComponentCreated() 
{
	Super::OnComponentCreated();
	Owner = Cast<ACharacterBase>(GetOwner());
	check(Owner != nullptr && "No Owner!")
	if(Owner)
	{	
		TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
		Owner->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);

		for (auto &comp : SkeletalMeshComponents)
		{

			//TODO: Add others
		}
	}
}

bool UEquipmentComponent::EquipHeadItem(UItemArmor *item)
{
	//if(item && CurrentWeight + item->Weight <= MaxWeight)
	SetUpdate();	// todo добавить общий вызов апдейта при манипуляции со снарягой
	return true;
}

void UEquipmentComponent::CalculateWeight()
{
	CurrentWeight = 0;
	for(int i = 0; i < int(EItemSlot::Max) - 1; ++i)
	{
		const UItemArmor* currentItem = Slot[i].Item;
		if(currentItem != nullptr)
			CurrentWeight += currentItem->Weight;
	}
}



void UEquipmentComponent::UpWeight()
{
	if(CurrentWeight < 1.5f * Owner->MaxWeight)
		CurrentWeight += 0.1f * Owner->MaxWeight;
	SetUpdate();
}

void UEquipmentComponent::DownWeight()
{
	if(CurrentWeight > 0 * Owner->MaxWeight)
		CurrentWeight -= 0.1f * Owner->MaxWeight;
	else
		CurrentWeight = 0;
	SetUpdate();
}
