// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Attributes/Attributes.h"
#include "Attributes/Skills.h"
#include "Items/ItemArmor.h"
#include "Items/ItemBase.h"
#include "Items/ItemWeapon.h"
#include "AbilitySystem/EffectBase.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "EquipmentComponent.generated.h"



USTRUCT(BlueprintType)
struct FEquipmentSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Equipment Slot")
    TSubclassOf<UItem> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Slot")
    FName SocketName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Slot")
    FTransform RelativeTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
    AItemActor *Item;
};



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MADFIRE_API UEquipmentComponent : public UActorComponent
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

    void SwapWeapons();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Component")
    float CurrentWeight;

    UFUNCTION(BlueprintCallable, Category = "Equipment Component")
    void Update();

    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void SetUpdate() { IsDirty = true; }
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void UnSetUpdate() { IsDirty = false; }
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    bool IsUpdateNeeded() { return IsDirty; }

    UPROPERTY(EditAnywhere, Category = "Equipment Component")
    FEquipmentSlot Slot[(int32)EItemSlot::Max];

private:
    bool IsDirty = false;

    void EquipAttach(EItemSlot SlotName);

private:    //Methods
    bool EquipHeadItem(UItemArmor *item);
    void CalculateWeight();
};
