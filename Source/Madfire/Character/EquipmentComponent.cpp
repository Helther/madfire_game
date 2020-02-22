// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../TraceFunctional/TraceDefines.h"

// Sets default values for this component's properties
/*UEquipmentComponent::UEquipmentComponent() :
	Owner(nullptr)
{}*/

void UEquipmentComponent::update()
{

}

void UEquipmentComponent::OnComponentCreated() 
{
	Super::OnComponentCreated();
	Owner = Cast<ACharacter>(GetOwner());

	if (Owner)
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
	return true;
}