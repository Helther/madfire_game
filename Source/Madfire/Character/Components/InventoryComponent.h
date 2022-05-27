// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Items/ItemBase.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "InventoryComponent.generated.h"


USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot")
    int32 Count = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot")
    UItem *Item = nullptr;
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();
    void OnComponentCreated() override;

    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    static bool SlotIsEmpty(UPARAM(ref) FInventorySlot &slot) { return slot.Item == nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    static bool RemoveFromSlot(UPARAM(ref) FInventorySlot &slot, int32 amount, bool removeWholeStack);

    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    static bool MergeSlots(UPARAM(ref) FInventorySlot &SlotA, UPARAM(ref) FInventorySlot &SlotB);

    UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    static bool SwapSlots(UPARAM(ref) FInventorySlot &SlotA, UPARAM(ref) FInventorySlot &SlotB);

	//New Inventory Interface
	UFUNCTION(BlueprintPure, Category = "Inventory Component")
    static int32 StacksFromAmount(int32 StackSize, int32 inAmount);

	UFUNCTION(BlueprintPure, Category = "Inventory Component")
    FORCEINLINE int32 GetInventorySize() const { return Inventory.Num(); }

	UFUNCTION(BlueprintPure, Category = "Inventory Component")
    int32 GetFreeInventorySlotsCount() const;

	UFUNCTION(BlueprintPure, Category = "Inventory Component")
    int32 GetItemCountByClass(UClass *itemClass) const;

	UFUNCTION(BlueprintPure, Category = "Inventory Component")
    bool GetStackByClass(UClass *itemClass, FInventorySlot &outSlot, int32 &foundIndex, bool returnFullStacks = false);

	UFUNCTION(BlueprintPure, Category = "Inventory Component")
    bool GetEmptySlot(FInventorySlot &outSlot, int32 &foundIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    bool InitializeInventory(int32 SlotsCount);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    FInventorySlot &GetSlotByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    bool SplitStack(int32 index, int32 splitCount);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    bool MergeStacks(int32 indexA, int32 indexB);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    bool SwapStacks(int32 indexA, int32 indexB);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    bool AddToInventory(UItem *Item, int32 amount, bool addNewStack = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    bool RemoveFromInventory(UClass *itemClass, int32 amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    bool RemoveFromStack(int32 index, int32 amount, bool removeWholeStack);

    UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    static bool InventoryEquip(UInventoryComponent *Inventory, int32 ItemIndex, class UEquipmentComponent *EquipComp, EItemSlot SlotName);

    UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    static bool MoveSlot(UInventoryComponent *FromInventory, UInventoryComponent *ToInventory, int32 FromIndex, int32 ToIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    bool UseItem(int32 ItemIndex, EInteractType interractType);

    UFUNCTION(BlueprintPure, Category = "Inventory Component")
    bool IsFull() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Component")
    float CurrentWeight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Component")
    TArray<FInventorySlot> Inventory;
		
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    void Update();

	UFUNCTION(BlueprintCallable, Category = "Component Update")
    void SetUpdate() { IsDirty = true; }
	UFUNCTION(BlueprintCallable, Category = "Component Update")
    void UnSetUpdate() { IsDirty = false; }
	UFUNCTION(BlueprintCallable, Category = "Component Update")
    bool IsUpdateNeeded() { return IsDirty; }

    UFUNCTION(BlueprintCallable, Category = "Inventory Component")
    bool DropItem(int32 index, int32 amount, FTransform transform);

private:
    static float WeightOfSlot(FInventorySlot &Slot);

    void CalculateWeight();
    bool IsDirty                = false;
    class ACharacterBase *Owner = nullptr;
};
