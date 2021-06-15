// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Attributes/Attributes.h"
#include "Attributes/Skills.h"
#include "Items/ItemArmor.h"
#include "Items/ItemBase.h"
#include "Items/ItemWeapon.h"
#include "Items/EquipmentSlot.h"
#include "AbilitySystem/EffectBase.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "EquipmentComponent.generated.h"




UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MADFIRE_API UEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEquipmentComponent(const FObjectInitializer& ObjectInitializer);

    void OnComponentCreated() override;

    UItem *Equip(UItem *Item);
    UItem *EquipToSlot(UItem *Item, EItemSlot SlotName);

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

	UFUNCTION(BlueprintCallable, Category = "Component Update")// todo 
    void SetUpdate() { IsDirty = true; }
	UFUNCTION(BlueprintCallable, Category = "Component Update")
    void UnSetUpdate() { IsDirty = false; }
	UFUNCTION(BlueprintCallable, Category = "Component Update")
    bool IsUpdateNeeded() { return IsDirty; }


	// debug
	UFUNCTION(BlueprintCallable, Category = "Equipment Component")
    void UpWeight();
	UFUNCTION(BlueprintCallable, Category = "Equipment Component")
    void DownWeight();

private:
    class ACharacterBase *Owner = nullptr;
    FEquipmentSlot Slot[(int32)EItemSlot::Max];
    bool IsDirty = false;

private:    //Methods
    bool EquipHeadItem(UItemArmor *item);
    void CalculateWeight();
};
