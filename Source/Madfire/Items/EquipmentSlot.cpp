#include "EquipmentSlot.h"

bool FEquipmentSlot::Equip(UItem *Item)
{
    if(Item == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipment: Item is Invalid"));
        return false;
    }

    if(!Item->IsA(ItemClass))
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipment: Item Class is Invalid"));
        return false;
    }

    this->Item = Item;
    return true;
}
