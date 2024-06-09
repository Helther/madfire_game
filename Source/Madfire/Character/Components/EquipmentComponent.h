// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Attributes/Attributes.h"
#include "Attributes/Skills.h"
#include "Items/ItemEquipment.h"
#include "AbilitySystem/StatusEffect.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "EquipmentComponent.generated.h"


struct FEquipmentSlotNames
{
    static const FName WeaponMain;
    static const FName WeaponOff;
    static const FName Shield;
    static const FName SheathMain;
    static const FName SheathOff;
    static const FName SheathWeaponBack;
    static const FName SheathShieldBack;
    static const FName Ammo;
    static const FName Head;
    static const FName Chest;
    static const FName Leg;
    static const FName Neck;
    static const FName RingLeft;
    static const FName RingRight;
};


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

USTRUCT(BlueprintType)
struct FUnarmedWeapon
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
    UItemArms* Weapon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AttachmentSlotName;
};



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEquipmentComponent(const FObjectInitializer& ObjectInitializer);

    void OnComponentCreated() override;

    static void AttachItemActor(ACharacter* Owner, AItemActor* ItemActor, FName OwnerSocketName, FName ItemSocketName);

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

    UFUNCTION(BlueprintCallable)
    AItemActor* SpawnItemActor(UItem* Item, bool EnableCollision, FName SocketName);

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Component")
    TArray<FUnarmedWeapon> DefaultArmaments;
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment Component")
    //TArray<USkeletalMesh*> EquippedMeshes;

    //Equipment Update
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void Update();

    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void SetUpdate(bool Update) { IsDirty = Update; }

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
