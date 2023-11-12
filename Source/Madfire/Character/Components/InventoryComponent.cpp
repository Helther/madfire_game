// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"

#include "TraceFunctional/TraceDefines.h"
#include "Character/CharacterBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Components/EquipmentComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent() {}

void UInventoryComponent::OnComponentCreated()
{
    Super::OnComponentCreated();
    Owner = Cast<ACharacterBase>(GetOwner());
    check(Owner != nullptr && "No Owner!");
}

bool UInventoryComponent::RemoveFromSlot(UPARAM(ref) FInventorySlot &slot, int32 amount,
                                         bool removeWholeStack)
{
    if (amount > slot.Count)
        return false;

    if (removeWholeStack || slot.Count == amount)
        slot = FInventorySlot();
    else
        slot.Count -= amount;
    return true;
}

bool UInventoryComponent::MergeSlots(UPARAM(ref) FInventorySlot &SlotA,
                                     UPARAM(ref) FInventorySlot &SlotB)
{
    if (SlotIsEmpty(SlotA) || SlotIsEmpty(SlotB))
        return false;
    if (!(SlotA.Item->GetClass() == SlotB.Item->GetClass()))
        return false;

    if (SlotA.Item->MaxCount >= SlotA.Count + SlotB.Count)
    {
        SlotA.Count += SlotB.Count;

        RemoveFromSlot(SlotB, 0, true);
        return true;
    }
    else
    {
        int32 remainder = SlotA.Item->MaxCount - SlotA.Count;
        SlotA.Count     = SlotA.Item->MaxCount;
        SlotB.Count -= remainder;

        return true;
    }
}

bool UInventoryComponent::SwapSlots(UPARAM(ref) FInventorySlot &SlotA, UPARAM(ref) FInventorySlot &SlotB)
{
    FInventorySlot tempSlot = SlotA;
    SlotA               = SlotB;
    SlotB               = tempSlot;
    return true;
}

float UInventoryComponent::WeightOfSlot(FInventorySlot &Slot)
{
    if (Slot.Item)
        return Slot.Item->Weight * Slot.Count;
    return 0;
}

int32 UInventoryComponent::StacksFromAmount(int32 StackSize, int32 inAmount)
{
    if (inAmount <= 0)
        return 0;

    if (StackSize <= 0)
        return 0;

    inAmount = FMath::DivideAndRoundUp(inAmount, StackSize);
    return inAmount;
}

int32 UInventoryComponent::GetFreeInventorySlotsCount() const
{
    int32 counter = 0;

    for (const auto &slot : Inventory)
        if (slot.Item == nullptr)
            ++counter;

    return counter;
}


int32 UInventoryComponent::GetItemCountByClass(UClass *itemClass) const
{
    int32 counter = 0;

    if (itemClass == nullptr)
        return 0;

    for (const auto &slot : Inventory)
    {
        if (slot.Item == nullptr)
            continue;
        if (slot.Item->GetClass() == itemClass)
            counter += slot.Count;
    }

    return counter;
}


bool UInventoryComponent::GetStackByClass(UClass *ItemClass, FInventorySlot &outSlot,
                                          int32 &foundIndex, bool returnFullStacks)
{
    for (auto Iterator(Inventory.CreateIterator()); Iterator; ++Iterator)
    {
        if ((Iterator->Item != nullptr) && (Iterator->Item->GetClass() == ItemClass))
        {
            if (Iterator->Count < Iterator->Item->MaxCount)
            {
                outSlot    = *Iterator;
                foundIndex = Iterator.GetIndex();

                return true;
            }
            else
            {
                if (returnFullStacks)
                {
                    outSlot    = *Iterator;
                    foundIndex = Iterator.GetIndex();

                    return true;
                }
            }
        }
    }

    return false;
}

bool UInventoryComponent::GetEmptySlot(FInventorySlot &outSlot, int32 &foundIndex) const
{
    for (auto Iterator(Inventory.CreateConstIterator()); Iterator; ++Iterator)
    {
        if (Iterator->Item == nullptr)
        {
            foundIndex = Iterator.GetIndex();
            outSlot    = *Iterator;

            return true;
        }
    }

    return false;
}

bool UInventoryComponent::InitializeInventory(int32 SlotsCount)
{
    Inventory = TArray<FInventorySlot>();
    Inventory.SetNum(SlotsCount);

    return true;
}

FInventorySlot &UInventoryComponent::GetSlotByIndex(int32 Index)
{
    check((Index >= 0 && Index < Inventory.Num()) && "Out of Range");

    return Inventory[Index];
}

bool UInventoryComponent::SplitStack(int32 index, int32 splitCount)
{
    FInventorySlot &slot = GetSlotByIndex(index);
    if (!slot.Item)
        return false;

    int32 remainder;
    const int32 freeSlots = GetFreeInventorySlotsCount();

    if (freeSlots < 1)
        return false;

    if ((splitCount >= slot.Count) || (splitCount <= 0))
        return false;

    auto newItemInstance = DuplicateObject(slot.Item, slot.Item->GetOuter());
    if (!newItemInstance)
        return false;

    remainder  = slot.Count - splitCount;
    slot.Count = remainder;

    return AddToInventory(newItemInstance, splitCount, true);
}

bool UInventoryComponent::MergeStacks(int32 indexA, int32 indexB)
{
    FInventorySlot &slotA = GetSlotByIndex(indexA);
    FInventorySlot &slotB = GetSlotByIndex(indexB);

    if (slotA.Item == nullptr || slotB.Item == nullptr)
        return false;
    if (slotA.Item->GetClass() != slotB.Item->GetClass())
        return false;

    if (slotA.Item->MaxCount >= slotA.Count + slotB.Count)
    {
        slotA.Count += slotB.Count;

        RemoveFromStack(indexB, 0, true);
        return true;
    }
    else
    {
        int32 remainder = slotA.Item->MaxCount - slotA.Count;
        slotA.Count     = slotA.Item->MaxCount;
        slotB.Count -= remainder;

        return true;
    }
}

bool UInventoryComponent::SwapStacks(int32 indexA, int32 indexB)
{
    const FInventorySlot tempSlot = Inventory[indexA];
    Inventory[indexA]             = Inventory[indexB];
    Inventory[indexB]             = tempSlot;
    return true;
}

bool UInventoryComponent::AddToInventory(UItem *Item, int32 amount, bool addNewStack)
{
    bool result;
    int32 foundStack = -1;

    if (amount <= 0 || Item == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Add empty item"));
        return false;
    }

    const int32 stackSize = Item->MaxCount;

    if (!addNewStack)
    {        
        FInventorySlot slot;
        result = GetStackByClass(Item->GetClass(), slot, foundStack);

        if (result)    //if inventory already contain item of that class
        {
            UE_LOG(LogTemp, Display, TEXT("Found item"));
            if (stackSize < slot.Count + amount)    // if not enough space in slot
            {
                UE_LOG(LogTemp, Display, TEXT("Not enough space in slot"));

                int32 Remainder             = Item->MaxCount - slot.Count;
                Remainder                   = amount - Remainder;
                Inventory[foundStack].Count = stackSize;

                auto NewItemInstance = DuplicateObject(Item, Item->GetOuter());

                AddToInventory(NewItemInstance, Remainder);
                return true;
            }
            else    //if we have enough space in slot
            {
                UE_LOG(LogTemp, Display, TEXT("Add item to founded slot"));

                Inventory[foundStack].Count += amount;
                return true;
            }
        }
        else    // if not we create new stack
        {
            UE_LOG(LogTemp, Display, TEXT("Not Found item"));
            if (amount > stackSize)    //if we have more item then slot can contain
            {
                UE_LOG(LogTemp, Display, TEXT("Not enough space in one slot"));

                const int32 freeSlots = GetFreeInventorySlotsCount();

                if (StacksFromAmount(stackSize, amount))
                    return false;

                const int32 Remainder = amount - stackSize;
                FInventorySlot itemSlot;

                result = GetEmptySlot(itemSlot, foundStack);

                if (!result)
                    return false;

                Inventory[foundStack].Count = stackSize;
                Inventory[foundStack].Item  = Item;

                auto NewItemInstance = DuplicateObject(Item, Item->GetOuter());

                AddToInventory(NewItemInstance, Remainder);

                return true;
            }
            else    //otherwise
            {                
                UE_LOG(LogTemp, Display, TEXT("Need to Add new stack"));
                //FInventorySlot itemSlot;
                //result = GetStackByClass(Item->GetClass(), itemSlot, foundStack, true);

                //if (!result)
                {
                    //return false;
                }

                //Inventory[foundStack].Count = amount;
                //Inventory[foundStack].Item  = Item;

                return AddToInventory(Item, amount, true);
            }
        }
    }
    else    //if add new stack
    {
        UE_LOG(LogTemp, Display, TEXT("Add new stack"));
        int32 freeSlots = GetFreeInventorySlotsCount();

        if (StacksFromAmount(stackSize, amount) > freeSlots)
            return false;

        FInventorySlot inventorySlot;

        result = GetEmptySlot(inventorySlot, foundStack);

        if (stackSize >= amount)    // if enough space in slot
        {
            Inventory[foundStack].Item  = Item;
            Inventory[foundStack].Count = amount;

            return true;
        }
        else    // if not enough space in slot
        {
            const int32 Remainder = amount - stackSize;

            Inventory[foundStack].Item  = Item;
            Inventory[foundStack].Count = stackSize;

            auto NewItemInstance = DuplicateObject(Item, Item->GetOuter());
            AddToInventory(NewItemInstance, Remainder, true);
        }
    }
    return true;
}

bool UInventoryComponent::RemoveFromInventory(UClass *itemClass, int32 amount)
{
    const int32 Counter = GetItemCountByClass(itemClass);

    if (amount > Counter)
        return false;

    int32 slotIndex = -1;
    FInventorySlot slot;

    const auto result = GetStackByClass(itemClass, slot, slotIndex);

    if (!result)
        return false;

    if (slot.Count > amount)
    {
        Inventory[slotIndex].Item -= amount;
        return true;
    }
    else
    {
        const int32 Remainder = amount - slot.Count;
        Inventory[slotIndex].Item->ConditionalBeginDestroy();
        Inventory[slotIndex] = FInventorySlot();

        return RemoveFromInventory(itemClass, Remainder);
    }
}

bool UInventoryComponent::RemoveFromStack(int32 index, int32 amount,
                                          bool removeWholeStack)
{
    FInventorySlot slot = GetSlotByIndex(index);

    if (amount > slot.Count)
        return false;

    if (removeWholeStack || slot.Count == amount)
    {
        Inventory[index].Item->ConditionalBeginDestroy();
        Inventory[index] = FInventorySlot();
        return true;
    }
    else
    {
        Inventory[index].Count -= amount;
        return true;
    }
}



void UInventoryComponent::Update()
{
    // todo finish
    CalculateWeight();
    SetUpdate(false);
}

void UInventoryComponent::CalculateWeight()
{
    CurrentWeight = 0;
    for (auto &slot : Inventory)
        CurrentWeight += WeightOfSlot(slot);
}

bool UInventoryComponent::DropItem(int32 index, int32 amount, FTransform transform)
{
    auto owner = Cast<ACharacterBase>(GetOwner());
    check(owner != nullptr && "UInventoryComponent: No owner");

    auto &slot = GetSlotByIndex(index);

    if (!IsValid(slot.Item))
    {
        UE_LOG(LogTemp, Display, TEXT("No item to drop"));
        return false;
    }

    slot.Count -= amount;
    auto item = slot.Item;

    owner->DropItem(item,amount,transform);

    if (slot.Count <= 0)
    {
        slot.Count = 0;
        slot.Item->ConditionalBeginDestroy();
        slot.Item = nullptr;
    }

    return true;
}

bool UInventoryComponent::IsFull() const
{
    FInventorySlot slot;
    int id;
    return !GetEmptySlot(slot,id);
}

bool UInventoryComponent::MoveSlot(UInventoryComponent *FromInventory, UInventoryComponent *ToInventory, int32 FromIndex, int32 ToIndex)
{
    check(FromInventory != nullptr);
    check(ToInventory != nullptr);

    auto &FromSlot = FromInventory->GetSlotByIndex(FromIndex);
    auto &ToSlot = ToInventory->GetSlotByIndex(ToIndex);

    if(FromSlot.Item == ToSlot.Item)
    {
        UE_LOG(AbilitySystemLog, Warning, TEXT("Inventory: Same Object"));
        return false;
    }

    if(FromSlot.Item != nullptr && ToSlot.Item != nullptr)
    {
        if(FromSlot.Item->GetClass() == ToSlot.Item->GetClass())
        {
            const auto imc = FromSlot.Item->MaxCount;

            ToSlot.Count += FromSlot.Count;

            if(ToSlot.Count > imc)
            {
                auto reminder = ToSlot.Count - imc;
                ToSlot.Count = imc;

                if(ToInventory->AddToInventory(FromSlot.Item,FromSlot.Count))
                {
                    FromSlot.Item = nullptr;
                    FromSlot.Count = 0;
                    return true;
                }

                FromSlot.Count = reminder;
            }
            else
            {
                FromSlot.Item->ConditionalBeginDestroy();
                FromSlot.Item = nullptr;
                FromSlot.Count = 0;
            }

            return true;
        }
    }
    return SwapSlots(ToSlot,FromSlot);
}



bool UInventoryComponent::InventoryEquip(UInventoryComponent *Inventory, int32 ItemIndex, class UEquipmentComponent *EquipComp, EItemSlot SlotName)
{
    check(Inventory != nullptr);
    check(EquipComp != nullptr);
    auto &Slot = Inventory->GetSlotByIndex(ItemIndex);

    auto Item = Slot.Item;
    auto EquipItem = EquipComp->EquipToSlot(Item,SlotName);

    if(Item == EquipItem)
        return false;

    if(Item != nullptr)
        Slot.Count--;

    if(EquipItem != nullptr)
    {
        if(Slot.Count <= 0)
        {
            Slot.Count = 1;
            Slot.Item = EquipItem;
        }
        else
        {
            Inventory->AddToInventory(EquipItem,1);
        }
    }
    else
    {
        Slot.Item = nullptr;
        Slot.Count = 0;
    }

    return true;
}


bool UInventoryComponent::UseItem(int32 ItemIndex, EInteractType interractType)
{
    auto owner = Cast<ACharacterBase>(GetOwner());
    check(owner != nullptr && "UInventoryComponent: No owner");
    auto &slot = GetSlotByIndex(ItemIndex);

    if(slot.Count == 0 || slot.Item == nullptr)
    {
        return false;
    }

    auto Item = slot.Item;
    auto result = Item->OnInteract(interractType,owner);

    if((Item->ItemFlags & (1 << int32(EItemFlags::Consumable))) && result && interractType == EInteractType::Use)
    {
        slot.Count--;
        if(slot.Count <=0)
        {
            slot.Count = 0;
            slot.Item = nullptr;
        }
    }
    return result;
}
