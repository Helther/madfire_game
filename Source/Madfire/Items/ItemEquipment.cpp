#include "ItemEquipment.h"


UItemChestArmor::UItemChestArmor(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{}

UItemLegsArmor::UItemLegsArmor(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{}

UItemGloves::UItemGloves(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{}

UItemFoot::UItemFoot(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{}

UItemHelmet::UItemHelmet(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{}

UItemNeck::UItemNeck(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{}

UItemRing::UItemRing(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{}


UItemWeapon::UItemWeapon(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{}

UItemShield::UItemShield(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{
    WeaponType = EWeaponType::Shield;
}
