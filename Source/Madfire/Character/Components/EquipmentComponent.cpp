// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentComponent.h"

#include "TraceFunctional/TraceDefines.h"
#include "Character/CharacterCreature.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties

void UEquipmentComponent::Update()
{
    // todo обновить бонусы к аттрибутам от снаряжения
    // проверить что содержимое слота изменилось
    //CalculateWeight(); debug temp
    auto Owner = Cast<ACharacterCreature>(GetOwner());
    check(Owner != nullptr && "No Owner!");
    Owner->AttributeComponent->Attributes += EquipmentAttributes;
    Owner->AttributeComponent->Skills += EquipmentSkills;
    Owner->AttributeComponent->DamageAbsorb += EqipmentDamageAbsorb;
    Owner->AttributeComponent->DamageResistance += EqipmentDamageResistance;
    Owner->AttributeComponent->DamageBonus += EqipmentDamageBonus;

    UnSetUpdate();
}

UEquipmentComponent::UEquipmentComponent(const FObjectInitializer &ObjectInitializer)
{
    Slot[(int32)EItemSlot::MainWeaponSlot].ItemClass = TSubclassOf<UItemWeapon>();
    Slot[(int32)EItemSlot::MainWeaponSlot].SocketName = "WeaponMainSlot";
    Slot[(int32)EItemSlot::OffMeaponSlot].ItemClass = TSubclassOf<UItemWeapon>();
    Slot[(int32)EItemSlot::OffMeaponSlot].SocketName = "WeaponOffSlot";
    Slot[(int32)EItemSlot::MainWeaponSlotSet2].ItemClass = TSubclassOf<UItemWeapon>();
    Slot[(int32)EItemSlot::MainWeaponSlotSet2].SocketName = "WeaponMainSlot";
    Slot[(int32)EItemSlot::OffWeaponSlotSet2].ItemClass = TSubclassOf<UItemWeapon>();
    Slot[(int32)EItemSlot::OffWeaponSlotSet2].SocketName = "WeaponOffSlot";

    Slot[(int32)EItemSlot::HeadSlot].ItemClass = UItemArmor::StaticClass();
    Slot[(int32)EItemSlot::HeadSlot].SocketName = "S_EquipHead";
}

void UEquipmentComponent::OnComponentCreated()
{
    Super::OnComponentCreated();
}

UItem *UEquipmentComponent::Equip(UItem *Item)
{
    //Try to deduce item place
    UE_LOG(LogTemp, Warning, TEXT("Equipment: Equip not working for now, use EquipToSlot"));
    return nullptr;
}

UItem *UEquipmentComponent::EquipToSlot(UItem *Item, EItemSlot SlotName)
{
    auto &slot = Slot[(int32)SlotName];

    if(!Item)
    {
        return Unequip(SlotName);
    }

    if(!Item->IsA(slot.ItemClass))
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipment: No class"));
        return Item;
    }

    AItemActor *itemActor = GetWorld()->SpawnActor<AItemActor>(Item->ItemActor);
    itemActor->SetOwner(GetOwner());
    check(itemActor != nullptr && "ItemActor is invalid");
    itemActor->ItemBase = Item;
    Item->ItemActor = itemActor->GetClass();

    itemActor->DisableComponentsSimulatePhysics();

    //TODO: SET Physics preset for Equipment

    auto tmp = Unequip(SlotName);

    Slot[(int32)SlotName].Item = itemActor;
    EquipAttach(SlotName);

    return tmp;
}


UItem *UEquipmentComponent::Unequip(EItemSlot SlotName)
{
    auto &slot = Slot[(int32)SlotName];

    if(!slot.Item)
    {
        return nullptr;
    }

    auto tmp = slot.Item->ItemBase;
    bool test = slot.Item->Destroy();

    slot.Item = nullptr;

    if(!test)
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipment: Item is Indestructible"));
    }

    return tmp;
}

FEquipmentSlot &UEquipmentComponent::GetEquipmentFromSlot(EItemSlot SlotName)
{
    return Slot[(int32)SlotName];
}

bool UEquipmentComponent::SwapItems(EItemSlot lhs, EItemSlot rhs)
{
    if(Slot[(int32)lhs].ItemClass == Slot[(int32)rhs].ItemClass)
    {        
        auto ownerMesh = Cast<ACharacterCreature>(GetOwner())->GetMesh();

        auto tmp = Slot[(int32)lhs].Item;
        Slot[(int32)lhs].Item = Slot[(int32)rhs].Item;
        Slot[(int32)rhs].Item = tmp;

        EquipAttach(lhs);
        EquipAttach(rhs);

        return true;
    }

    return false;
}

void UEquipmentComponent::SwapWeapons()
{
    //
}

void UEquipmentComponent::CalculateWeight()
{
    CurrentWeight = 0;
    for (int i = 0; i < int(EItemSlot::Max) - 1; ++i)
    {
        if ((Slot[i].Item != nullptr) && (Slot[i].Item->ItemBase != nullptr))
            CurrentWeight += Slot[i].Item->ItemBase->Weight;
    }
}

void UEquipmentComponent::EquipAttach(EItemSlot SlotName)
{
    auto ownerMesh = Cast<ACharacterCreature>(GetOwner())->GetMesh();
    check(ownerMesh != nullptr && "OwnerMesh is invalid");
    auto &slot = Slot[(int32)SlotName];

    if(slot.Item)
        slot.Item->AttachToComponent(ownerMesh,FAttachmentTransformRules::SnapToTargetNotIncludingScale,slot.SocketName);
}
