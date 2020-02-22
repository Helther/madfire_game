// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Items/ItemBase.h"
#include "../Items/ItemArmor.h"
#include "../Items/ItemWeapon.h"
#include "../Attributes/Attributes.h"
#include "../Attributes/Skills.h"
#include "EquipmentComponent.generated.h"



USTRUCT(BlueprintType)
struct FEquipmentSlot
{
	GENERATED_BODY()
		
	UPROPERTY(BlueprintReadWrite, Category = "Equipment Slot")
		EItemSlot SlotType;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Equipment Slot")
		USkeletalMeshComponent *AttachedMeshSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
		UItemArmor *Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Slot")
		FTransform RelativeTransform;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	void OnComponentCreated() override;

	void update();
	
	/*
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Component")
		class UItemWeapon *MainWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Component")
		class UItemWeapon *OffHandWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Component")
		class UItemWeapon *MainWeaponSet2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Component")
		class UItemWeapon *OffHandWeaponSet2;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Component")
		class UItemArmor *HeadSlot;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Component")
		class UItemArmor *ChestSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Component")
		class UItemArmor *FootSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Component")
		class UItemArmor *LegSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Component")
		class UItemArmor *HandsSlot;
		*/

	UPROPERTY(BlueprintReadOnly, Category = "Equipment Component")
		FSkillList EquipmentSkills;
	UPROPERTY(BlueprintReadOnly, Category = "Equipment Component")
		FAttributes EquimpentAttributes;
	UPROPERTY(BlueprintReadOnly, Category = "Equipment Component")
		FDamageAbsorb  EqipmentDamageAbsorb;
	UPROPERTY(BlueprintReadOnly, Category = "Equipment Component")
		FDamageResistance  EqipmentDamageResistance;
	UPROPERTY(BlueprintReadOnly, Category = "Equipment Component")
		FDamageAbsorb DamageBonus;
	//UPROPERTY(BlueprintReadOnly, Category = "Equipment Component")
		//Effects

private:
	ACharacter *Owner = nullptr;
    FEquipmentSlot Slot[(int32)EItemSlot::Max];
	

private: //Methods
	bool EquipHeadItem(UItemArmor *item);

};
