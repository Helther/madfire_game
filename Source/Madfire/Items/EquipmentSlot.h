#pragma once

#include "CoreMinimal.h"
#include "Items/ItemArmor.h"
#include "Items/ItemWeapon.h"
#include "EquipmentSlot.generated.h"

USTRUCT(BlueprintType)
struct FEquipmentSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
    USkeletalMeshComponent *AttachedMeshSlot;

    UPROPERTY(BlueprintReadOnly, Category = "Equipment Slot")
    TSubclassOf<UItem> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
    UItem *Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
    FTransform RelativeTransform;

    bool Equip(UItem *Item);

    template<typename ItemClass>
    ItemClass GetItem() const {return Cast<ItemClass>(Item);}
};
