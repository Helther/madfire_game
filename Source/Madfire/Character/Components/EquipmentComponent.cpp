// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentComponent.h"

#include "TraceFunctional/TraceDefines.h"
#include "Character/CharacterCreature.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
/*UEquipmentComponent::UEquipmentComponent() :
	Owner(nullptr)
{}*/

void UEquipmentComponent::Update()
{
    // todo обновить бонусы к аттрибутам от снаряжения
    // проверить что содержимое слота изменилось
    //CalculateWeight(); debug temp
    Owner->AttributeComponent->Attributes += EquipmentAttributes;
    Owner->AttributeComponent->Skills += EquipmentSkills;
    Owner->AttributeComponent->DamageAbsorb += EqipmentDamageAbsorb;
    Owner->AttributeComponent->DamageResistance += EqipmentDamageResistance;
    Owner->AttributeComponent->DamageBonus += EqipmentDamageBonus;

    UnSetUpdate();
}

UEquipmentComponent::UEquipmentComponent(const FObjectInitializer &ObjectInitializer)
{

}

void UEquipmentComponent::OnComponentCreated()
{
    Super::OnComponentCreated();
    Owner = Cast<ACharacterCreature>(GetOwner());
    check(Owner != nullptr && "No Owner!");
    if (Owner)
    {
        TArray<USkeletalMeshComponent *> SkeletalMeshComponents;
        Owner->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);

        for (auto &comp : SkeletalMeshComponents)
        {
            //TODO: Add others
        }
    }
}

UItem *UEquipmentComponent::Equip(UItem *Item)
{
    //Try to deduce item place
    return nullptr;
}

UItem *UEquipmentComponent::EquipToSlot(UItem *Item, EItemSlot SlotName)
{
    auto tmp = Slot[(int32)SlotName].Item;
    Slot[(int32)SlotName].Item = Item;
    return tmp;
}

FEquipmentSlot* UEquipmentComponent::GetEquipmentFromSlot(EItemSlot SlotName)
{
    return &Slot[(int32)SlotName];
}

void UEquipmentComponent::SwapWeapons()
{
    //
}

bool UEquipmentComponent::EquipHeadItem(UItemArmor *item)
{
    //if(item && CurrentWeight + item->Weight <= MaxWeight)
    SetUpdate();    // todo добавить общий вызов апдейта при манипуляции со снарягой
    return true;
}

void UEquipmentComponent::CalculateWeight()
{
    CurrentWeight = 0;
    for (int i = 0; i < int(EItemSlot::Max) - 1; ++i)
    {
        const UItem *currentItem = Slot[i].Item;
        if (currentItem != nullptr)
            CurrentWeight += currentItem->Weight;
    }
}


void UEquipmentComponent::UpWeight()
{
    if (CurrentWeight < 1.5f * Owner->MaxWeight)
        CurrentWeight += 0.1f * Owner->MaxWeight;
    SetUpdate();
}

void UEquipmentComponent::DownWeight()
{
    if (CurrentWeight > 0 * Owner->MaxWeight)
        CurrentWeight -= 0.1f * Owner->MaxWeight;
    else
        CurrentWeight = 0;
    SetUpdate();
}
