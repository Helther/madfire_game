// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentComponent.h"

#include "TraceFunctional/TraceDefines.h"
#include "Character/CharacterCreature.h"
#include "Character/CharacterAnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Components/CharacterMeshComponent.h"


const FName FEquipmentSlotNames::WeaponMain = "S_WeaponMain";
const FName FEquipmentSlotNames::WeaponOff = "S_WeaponOff";
const FName FEquipmentSlotNames::Shield = "S_Shield";
const FName FEquipmentSlotNames::SheathMain = "S_WeaponSheathMain";
const FName FEquipmentSlotNames::SheathOff = "S_WeaponSheathOff";
const FName FEquipmentSlotNames::SheathWeaponBack = "S_WeaponSheathBack";
const FName FEquipmentSlotNames::SheathShieldBack = "S_ShieldSheathBack";
const FName FEquipmentSlotNames::Ammo = "S_Ammo";
const FName FEquipmentSlotNames::Head = "S_EquipHead";
const FName FEquipmentSlotNames::Chest = "S_EquipChest";
const FName FEquipmentSlotNames::Leg = "S_EquipLeg";
const FName FEquipmentSlotNames::Neck = "S_EquipNeck";
const FName FEquipmentSlotNames::RingLeft = "S_EquipRingLeft";
const FName FEquipmentSlotNames::RingRight = "S_EquiRingRight";


// Sets default values for this component's properties

void UEquipmentComponent::Update()
{
    // todo обновить бонусы к аттрибутам от снаряжения
    // проверить что содержимое слота изменилось
    //CalculateWeight(); debug temp
    auto Owner = Cast<ACharacterCreature>(GetOwner());
    check(Owner != nullptr && "No Owner!");
    Owner->GetAttributeComponent()->Attributes += EquipmentAttributes;
    Owner->GetAttributeComponent()->Skills += EquipmentSkills;
    Owner->GetAttributeComponent()->DamageAbsorb += EqipmentDamageAbsorb;
    Owner->GetAttributeComponent()->DamageResistance += EqipmentDamageResistance;
    Owner->GetAttributeComponent()->DamageBonus += EqipmentDamageBonus;

    SetUpdate(false);
}

UEquipmentComponent::UEquipmentComponent(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    Slot[(int32)EItemSlot::MainWeaponSlot].ItemClass = UItemArms::StaticClass();
    Slot[(int32)EItemSlot::MainWeaponSlot].SocketName = FEquipmentSlotNames::WeaponMain;
    Slot[(int32)EItemSlot::OffMeaponSlot].ItemClass = UItemArms::StaticClass();
    Slot[(int32)EItemSlot::OffMeaponSlot].SocketName = FEquipmentSlotNames::WeaponOff;
    Slot[(int32)EItemSlot::MainWeaponSlotSet2].ItemClass = UItemArms::StaticClass();
    Slot[(int32)EItemSlot::MainWeaponSlotSet2].SocketName = FEquipmentSlotNames::WeaponMain;
    Slot[(int32)EItemSlot::OffWeaponSlotSet2].ItemClass = UItemArms::StaticClass();
    Slot[(int32)EItemSlot::OffWeaponSlotSet2].SocketName = FEquipmentSlotNames::WeaponOff;
    Slot[(int32)EItemSlot::AmmoSlot].ItemClass = UItemAmmo::StaticClass();
    Slot[(int32)EItemSlot::AmmoSlot].SocketName = FEquipmentSlotNames::Ammo;

    Slot[(int32)EItemSlot::HeadSlot].ItemClass = UItemHelmet::StaticClass();
    Slot[(int32)EItemSlot::ChestSlot].ItemClass = UItemChestArmor::StaticClass();
    Slot[(int32)EItemSlot::FootSlot].ItemClass = UItemFoot::StaticClass();
    Slot[(int32)EItemSlot::HandsSlot].ItemClass = UItemGloves::StaticClass();
    Slot[(int32)EItemSlot::LegSlot].ItemClass = UItemLegsArmor::StaticClass();

    Slot[(int32)EItemSlot::NeckSlot].ItemClass = UItemNeck::StaticClass();
    Slot[(int32)EItemSlot::RightRingSlot].ItemClass = UItemRing::StaticClass();
    Slot[(int32)EItemSlot::LeftRingSlot].ItemClass = UItemRing::StaticClass();
}

void UEquipmentComponent::OnComponentCreated()
{
    Super::OnComponentCreated();
}

void UEquipmentComponent::AttachItemActor(ACharacter* Owner, AItemActor* ItemActor, FName OwnerSocketName, FName ItemSocketName)
{
    if (ItemActor->ItemMesh->DoesSocketExist(ItemSocketName))
    {
        FTransform socketTransform = ItemActor->ItemMesh->GetSocketTransform(ItemSocketName);
        FVector relativeLocation = socketTransform.InverseTransformPosition(ItemActor->GetActorLocation());
        FQuat relativeRot = socketTransform.InverseTransformRotation(ItemActor->GetActorQuat());
        ItemActor->SetActorRelativeLocation(relativeLocation);
        ItemActor->SetActorRelativeRotation(relativeRot);
    }
    else
    {
        ItemActor->SetActorRelativeLocation(FVector::ZeroVector);
        ItemActor->SetActorRelativeRotation(FQuat::Identity);
    }
    FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, false);
    ItemActor->AttachToComponent(Owner->GetMesh(), rules, OwnerSocketName);
}

UItem *UEquipmentComponent::Equip(UItem *Item)
{
    //Try to deduce item place
    UE_LOG(LogTemp, Warning, TEXT("Equipment: Equip not working for now, use EquipToSlot"));
    return nullptr;
}

UItem *UEquipmentComponent::EquipToSlot(UItem *Item, EItemSlot SlotName)
{
    // check in widgets EquipFromInventory if combat is busy before calling that
    auto owner = Cast<ACharacterCreature>(GetOwner());
    if (owner && owner->GetCombatComponent()->IsBusy())
        return Item;
    auto &slot = Slot[(int32)SlotName];

    if(!Item)
    {
        auto tmp = Unequip(SlotName);
        EquipmentSlotUpdated.Broadcast(this, SlotName);
        check(tmp && "EquipToSlot: No item");
        return tmp;
    }

    auto EquipItem = Cast<UItemEquipable>(Item);

    if(!EquipItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipment: No class"));
        return Item;
    }

    if(!Item->IsA(slot.ItemClass))
    {  // item can be of subclass, so check if the slot is correct
        auto weapon = Cast<UItemWeapon>(Item);
        auto shield = Cast<UItemShield>(Item);
        if (!weapon)
        {
            if (!shield)
            {
                UE_LOG(LogTemp, Warning, TEXT("EquipToSlot: No class"));
                return Item;
            }
            else if ((SlotName != EItemSlot::OffMeaponSlot) && (SlotName != EItemSlot::OffWeaponSlotSet2))
            {
                UE_LOG(LogTemp, Warning, TEXT("EquipToSlot: invalid slot for shield"));
                return Item;
            }
        }
        else
        {
            if ((SlotName != EItemSlot::MainWeaponSlot) && (SlotName != EItemSlot::OffMeaponSlot) && 
                (SlotName != EItemSlot::MainWeaponSlotSet2) && (SlotName != EItemSlot::OffWeaponSlotSet2))
            {
                UE_LOG(LogTemp, Warning, TEXT("EquipToSlot: invalid slot for weapon"));
                return Item;
            }
        }
    }
    AItemActor *itemActor = SpawnItemActor(Item, false, slot.SocketName);
    auto tmp = Unequip(SlotName);
    Slot[(int32)SlotName].Item = itemActor;
    bool shouldMerge = itemActor && itemActor->IsA(AItemActorEquipmentMerge::StaticClass());
    if (shouldMerge)
    {
        owner->GetMeshComponent()->SetUpdate(true);
        owner->Update(FCharacterLevelInfo());
    }
    else
    {
        EquipAttach(SlotName);
    }
    EquipItem->OnEquip(this);
    EquipmentSlotUpdated.Broadcast(this, SlotName);

    //TODO: Check One Weapon MainHand only

    return tmp;
}


UItem *UEquipmentComponent::Unequip(EItemSlot SlotName)
{
    auto &slot = Slot[(int32)SlotName];

    if(!slot.Item)
    {
        return nullptr;
    }
    bool wasMegredWithCharacterMesh = slot.Item->IsA(AItemActorEquipmentMerge::StaticClass());
    auto tmp = slot.Item->ItemBase;
    bool test = slot.Item->Destroy();

    slot.Item = nullptr;

    if(!test)
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipment: Item is Indestructible"));
    }
    auto owner = Cast<ACharacterCreature>(GetOwner());
    auto EquipItem = Cast<UItemEquipable>(tmp);
    if(EquipItem)
    {
        EquipItem->OnUnequip(this);
    }
    EquipmentSlotUpdated.Broadcast(this, SlotName);
    if (owner && wasMegredWithCharacterMesh)
    {
        owner->GetMeshComponent()->SetUpdate(true);
        owner->Update(FCharacterLevelInfo());
    }

    return tmp;
}

bool UEquipmentComponent::DropItem(EItemSlot SlotName, FTransform transform)
{
    auto owner = Cast<ACharacterBase>(GetOwner());
    check(owner != nullptr && "Equipment: No owner");

    auto item = Unequip(SlotName);

    if(!IsValid(item))
    {
        UE_LOG(LogTemp, Display, TEXT("No item to drop"));
        return false;
    }

    owner->DropItem(item,1,transform);
    return true;
}

FEquipmentSlot &UEquipmentComponent::GetEquipmentFromSlot(EItemSlot SlotName)
{
    return Slot[(int32)SlotName];
}

bool UEquipmentComponent::SwapItems(EItemSlot lhs, EItemSlot rhs)
{
    auto owner = Cast<ACharacterCreature>(GetOwner());
    check(owner != nullptr && "Equipment: No owner");
    if(Slot[(int32)lhs].ItemClass == Slot[(int32)rhs].ItemClass && owner->GetCombatComponent()->IsWeaponSwapAllowed(lhs, rhs))
    {
        auto ownerMesh = owner->GetMesh();

        auto tmp = Slot[(int32)lhs].Item;
        Slot[(int32)lhs].Item = Slot[(int32)rhs].Item;
        Slot[(int32)rhs].Item = tmp;

        EquipAttach(lhs);
        EquipAttach(rhs);
        EquipmentSwaped.Broadcast(this, lhs, rhs);
        return true;
    }

    return false;
}

AItemActor* UEquipmentComponent::SpawnItemActor(UItem* Item, bool EnableCollision, FName SocketName)
{
    FVector socketLoc = Cast<ACharacterCreature>(GetOwner())->GetMesh()->GetSocketLocation(SocketName);
    AItemActor *itemActor = Cast<AItemActor>(GetWorld()->SpawnActor(Item->ItemActorInEquipment, &socketLoc));
    check(itemActor != nullptr && "ItemActor is invalid");
    itemActor->SetOwner(GetOwner());
    itemActor->ItemBase = Item;
    ACharacterBase::SetupItemActorCollision(itemActor, EnableCollision);
    return itemActor;
}

void UEquipmentComponent::CalculateWeight()
{
    CurrentWeight = 0;
    for (int i = 0; i < int(EItemSlot::Max) - 1; ++i)
    {
        if ((Slot[i].Item != nullptr) && (Slot[i].Item->ItemBase != nullptr))
            CurrentWeight += Slot[i].Item->ItemBase->Weight;
    }
}

void UEquipmentComponent::EquipAttach(EItemSlot SlotName)
{
    auto owner = Cast<ACharacterCreature>(GetOwner());
    check(owner && "Owner is invalid");
    auto &slot = Slot[(int32)SlotName];

    if (slot.Item)
    {
        FName socketName = slot.SocketName;
        if (SlotName == EItemSlot::MainWeaponSlot || SlotName == EItemSlot::OffMeaponSlot)
        {// attach weapon to the actor as fallback before updating stance, and then stance might reattach it elsewhere
            auto arms = Cast<UItemArms>(slot.Item->ItemBase);
            check(arms && "invalid weapon item");
            socketName = UCombatComponent::GetSlotFromWeaponType(arms->WeaponType, false, SlotName == EItemSlot::MainWeaponSlot);
        }
        AttachItemActor(owner, slot.Item, socketName, FItemActorEquipmentSlotNames::EquipSocket);
    }
}

