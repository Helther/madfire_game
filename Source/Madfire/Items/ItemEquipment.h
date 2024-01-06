#pragma once

#include "Items/ItemBase.h"
#include "../Rules/GameRules.h"
#include "Attributes/DamageMap.h"
#include "Attributes/Damage.h"
#include "AbilitySystem/AbstractAbility.h"
#include "ItemEquipment.generated.h"


class UEquipmentComponent;
class ACharacterCreature;

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

    void Tick(float DeltaSeconds, const ACharacterCreature* Owner);
    UFUNCTION(BlueprintCallable)
    void AddSpread();

    /* Returns the current spread angle (in degrees, diametrical) */
    UFUNCTION(BlueprintCallable)
    float GetCalculatedSpreadAngle() const { return CurrentSpreadAngle; }
    UFUNCTION(BlueprintCallable)
    float GetCalculatedSpreadAngleMultiplier() const { return CurrentSpreadAngleMultiplier; }
    double GetLastAddSpreadTime() const { return LastAddSpreadTime; }

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


    // min/max heat params
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread|Fire Params", Meta = (UIMin = "0.1", ClampMin = "0.1"))
    float HeatMin = 2.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread|Fire Params", Meta = (UIMin = "0.1", ClampMin = "0.1"))
    float HeatMax = 20.f;
    // how heat property convert to spread angle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread|Fire Params", Meta = (UIMin = "0.01", ClampMin = "0.01"))
    float HeatToSpreadMultiplier = 1.f;
    // cooldown rate param per second
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread|Fire Params", Meta = (UIMin = "0.01", ClampMin = "0.01"))
    float HeatCooldownRate = 10.f;
    // amount of heat to add when introducing instability while aiming (refocucing)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread|Fire Params", Meta = (UIMin = "0.00", ClampMin = "0.00"))
    float HeatAddDueToInstabilityBase = 5.f;

    // Time since firing before spread cooldown recovery begins (in seconds)
    UPROPERTY(EditAnywhere, Category="Spread|Fire Params", meta=(ForceUnits=s))
    float SpreadRecoveryCooldownDelay = 0.0f;


    // TODO implement Multiplier when in an aiming camera mode
    UPROPERTY(EditAnywhere, Category="Spread|Player Params", meta=(ForceUnits=x))
    float SpreadAngleMultiplier_Aiming = 1.0f;

    // Multiplier when standing still or moving very slowly
    // (starts to fade out at StandingStillSpeedThreshold, and is gone completely by StandingStillSpeedThreshold + StandingStillToMovingSpeedRange)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
    float SpreadAngleMultiplier_StandingStill = 1.0f;
    // Multiplier when standing still or moving very fast
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
    float SpreadAngleMultiplier_Running = 2.0f;

    // Rate at which we transition to/from the standing still accuracy (higher values are faster, though zero is instant; @see FInterpTo)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
    float TransitionRate_StandingStill = 5.0f;

    // Speeds at or below this are considered standing still
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits="cm/s"))
    float StandingStillSpeedThreshold = 80.0f;

    // Speeds no more than this above StandingStillSpeedThreshold are used to feather down the standing still bonus until it's back to 1.0
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits="cm/s"))
    float StandingStillToMovingSpeedRange = 20.0f;

    // Multiplier when crouching, smoothly blended to based on TransitionRate_Crouching
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
    float SpreadAngleMultiplier_Crouching = 1.0f;

    // Rate at which we transition to/from the crouching accuracy (higher values are faster, though zero is instant; @see FInterpTo)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
    float TransitionRate_Crouching = 5.0f;

    // Spread multiplier while jumping/falling, smoothly blended to based on TransitionRate_JumpingOrFalling
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
    float SpreadAngleMultiplier_JumpingOrFalling = 2.0f;

    // Rate at which we transition to/from the jumping/falling accuracy (higher values are faster, though zero is instant; @see FInterpTo)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
    float TransitionRate_JumpingOrFalling = 5.0f;

private:
    inline float ClampHeat(float NewHeat)
    {
        return FMath::Clamp(NewHeat, HeatMin, HeatMax);
    }
    // Updates the spread and returns true if the spread is at minimum
    void UpdateSpread(float DeltaSeconds);
    // Updates the multipliers and returns true if they are at minimum
    void UpdateMultipliers(float DeltaSeconds, const ACharacterCreature* Owner);

    // Time since this weapon was last fired (relative to world time)
    double LastAddSpreadTime = 0.0;

    // The current heat
    float CurrentHeat = 0.0f;

    // The current spread angle (in degrees, diametrical)
    float CurrentSpreadAngle = 0.0f;

    // The current *combined* spread angle multiplier
    float CurrentSpreadAngleMultiplier = 1.0f;

    // The current standing still multiplier
    float StandingStillMultiplier = 1.0f;

    // The current jumping/falling multiplier
    float JumpFallMultiplier = 1.0f;

    // The current crouching multiplier
    float CrouchingMultiplier = 1.0f;
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
