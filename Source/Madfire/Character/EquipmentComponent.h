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
class MADFIRE_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	void OnComponentCreated() override;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Component")
		float CurrentWeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Component")
		float MaxWeight;

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

private: //Methods
	bool EquipHeadItem(UItemArmor *item);
	void CalculateWeight();	
	void CalculateMaxWeight();
};
