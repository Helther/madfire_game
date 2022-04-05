// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Attributes/Attributes.h"
#include "Attributes/Skills.h"
#include "Items/ItemEquipment.h"
#include "AbilitySystem/EffectBase.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "EquipmentComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipmentSlotEvent, class UEquipmentComponent*, Equipment, EItemSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEquipmentSwapEvent, class UEquipmentComponent*, Equipment, EItemSlot, SlotL, EItemSlot, SlotR);


USTRUCT(BlueprintType)
struct FEquipmentSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Equipment Slot")
    TSubclassOf<UItemEquipable> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Slot")
    FName SocketName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Slot")
    FTransform RelativeTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
    AItemActor *Item;
};



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEquipmentComponent(const FObjectInitializer& ObjectInitializer);

    void OnComponentCreated() override;

    UFUNCTION(BlueprintCallable, Category = "Equipment Component")
    UItem *Equip(UItem *Item);
    UFUNCTION(BlueprintCallable, Category = "Equipment Component")
    UItem *EquipToSlot(UItem *Item, EItemSlot SlotName);
    UFUNCTION(BlueprintCallable, Category = "Equipment Component")
    UItem *Unequip(EItemSlot SlotName);
    UFUNCTION(BlueprintCallable, Category = "Equipment Component")
    bool DropItem(EItemSlot SlotName, FTransform transform);
    //UFUNCTION(BlueprintCallable, Category = "Equipment Component")
    //bool EquipFromInventorySlot()

    UFUNCTION(BlueprintPure, Category = "Equipment Component")
    FEquipmentSlot &GetEquipmentFromSlot(EItemSlot SlotName);

    UFUNCTION(BlueprintCallable, Category = "Equipment Component")
    bool SwapItems(EItemSlot lhs, EItemSlot rhs);

    UPROPERTY(BlueprintReadOnly, Category = "Equipment Component")
    FSkillList EquipmentSkills;
    UPROPERTY(BlueprintReadOnly, Category = "Equipment Component")
    FAttributeSet EquipmentAttributes;
    UPROPERTY(BlueprintReadOnly, Category = "Equipment Component")
    FDamageAbsorb  EqipmentDamageAbsorb;
    UPROPERTY(BlueprintReadOnly, Category = "Equipment Component")
    FDamageResistance  EqipmentDamageResistance;
    UPROPERTY(BlueprintReadOnly, Category = "Equipment Component")
    FDamageAbsorb EqipmentDamageBonus;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Component")
    float CurrentWeight;
    UPROPERTY(EditAnywhere, Category = "Equipment Component")
    FEquipmentSlot Slot[(int32)EItemSlot::Max];

    //Equipment Update
    UFUNCTION(BlueprintCallable, Category = "Component Component")
    void Update();
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void SetUpdate() { IsDirty = true; }
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void UnSetUpdate() { IsDirty = false; }
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    bool IsUpdateNeeded() { return IsDirty; }

    // Equipment Events
    UPROPERTY(BlueprintAssignable, Category = "Equipment Component")
    FEquipmentSlotEvent EquipmentSlotUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Equipment Component")
    FEquipmentSwapEvent EquipmentSwaped;

private:
    bool IsDirty = false;

    void EquipAttach(EItemSlot SlotName);
    void CalculateWeight();
};
