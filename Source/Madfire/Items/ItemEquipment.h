#pragma once

#include "Items/ItemBase.h"
#include "../Rules/GameRules.h"
#include "Attributes/DamageMap.h"
#include "Attributes/Damage.h"
#include "AbilitySystem/AbstractAbility.h"
#include "ItemEquipment.generated.h"


class UEquipmentComponent;

UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemEquipable : public UItem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintImplementableEvent, Category = "Item Events")
    void OnEquip(class UEquipmentComponent *Causer);

    UFUNCTION(BlueprintImplementableEvent, Category = "Item Events")
    void OnUnequip(class UEquipmentComponent *Causer);
};


UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemChestArmor : public UItemEquipable
{
    GENERATED_BODY()

public:
    UItemChestArmor(const class FObjectInitializer &ObjectInititalizer);

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Weapon Stats")
    EArmorType ArmorType;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageAbsorb DamageLimit;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageResistance DamageResistance;
};

UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemLegsArmor : public UItemEquipable
{
    GENERATED_BODY()

public:
    UItemLegsArmor(const class FObjectInitializer &ObjectInititalizer);

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Weapon Stats")
    EArmorType ArmorType;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageAbsorb DamageLimit;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageResistance DamageResistance;
};

UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemGloves : public UItemEquipable
{
    GENERATED_BODY()

public:
    UItemGloves(const class FObjectInitializer &ObjectInititalizer);

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageAbsorb DamageLimit;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageResistance DamageResistance;
};

UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemFoot : public UItemEquipable
{
    GENERATED_BODY()

public:
    UItemFoot(const class FObjectInitializer &ObjectInititalizer);

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageAbsorb DamageLimit;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageResistance DamageResistance;
};

UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemHelmet : public UItemEquipable
{
    GENERATED_BODY()

public:
    UItemHelmet(const class FObjectInitializer &ObjectInititalizer);

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageAbsorb DamageLimit;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageResistance DamageResistance;
};

UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemNeck : public UItemEquipable
{
    GENERATED_BODY()

public:
    UItemNeck(const class FObjectInitializer &ObjectInititalizer);

};

UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemRing : public UItemEquipable
{
    GENERATED_BODY()

public:
    UItemRing(const class FObjectInitializer &ObjectInititalizer);

};



UCLASS(config = Game, BlueprintType)
class UItemArms : public UItemEquipable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
    EWeaponType WeaponType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
    EWeaponSize WeaponSize;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
    TSubclassOf<UPhysicDamage> DamageType;
};


UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemWeapon : public UItemArms
{
    GENERATED_BODY()

public:
    UItemWeapon(const class FObjectInitializer &ObjectInititalizer);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    float DamageValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageAbsorb DamageLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageResistance DamageResistance;
};

UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemWeaponRanged : public UItemWeapon
{
    GENERATED_BODY()

public:
    UItemWeaponRanged(const class FObjectInitializer &ObjectInititalizer);
    // tension scales linearly with projectile speed, this is a value for instant bow shooting when just started drawing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats", Meta = (UIMin = "0.0", ClampMin = "0.0", UIMax = "5000.0", ClampMax = "5000.0"))
    float MinTensionStrength = 500.f;
    // used as default speed for crossbows
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats", Meta = (UIMin = "0.0", ClampMin = "0.0", UIMax = "5000.0", ClampMax = "5000.0"))
    float MaxTensionStrength = 3000.f;
    // base speed modifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats", Meta = (UIMin = "0.1", ClampMin = "0.1", UIMax = "2.0", ClampMax = "2.0"))
    float TensionCoefficient = 1.f;
    // speed modifier from an attribute
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats", Meta = (UIMin = "0.1", ClampMin = "0.1", UIMax = "2.0", ClampMax = "2.0"))
    float AttributeTensionCoefficient = 1.f;
    // ammo damage modifier based on initial speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats", Meta = (UIMin = "0.01", ClampMin = "0.01", UIMax = "2.0", ClampMax = "2.0"))
    float AmmoDamageCoefficient = 0.1f;
};


UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemShield : public UItemArms
{
    GENERATED_BODY()

public:
    UItemShield(const class FObjectInitializer &ObjectInititalizer);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageAbsorb DamageLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
    FDamageResistance DamageResistance;
};

// TODO add effects on impact containter
UCLASS(config = Game, BlueprintType, Blueprintable)
class UItemAmmo : public UItemEquipable
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats", Meta = (UIMin = "0.0", ClampMin = "0.0"))
    float DamageValue;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
    TSubclassOf<UPhysicDamage> DamageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    TSubclassOf<AItemActorProjectile> ProjectileActor;
    // defines actor gravity coefficient, the more the heavier projectile becomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats", Meta = (UIMin = "0.1", ClampMin = "0.1", UIMax = "2.0", ClampMax = "2.0"))
    float WeightCoefficient = 1.f;
    // defines strike force modifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats", Meta = (UIMin = "0.1", ClampMin = "0.1", UIMax = "2.0", ClampMax = "2.0"))
    float ForceCoefficient = 1.f;
};
