#include "ItemEquipment.h"
#include "Character/CharacterCreature.h"

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

UItemWeaponRanged::UItemWeaponRanged(const class FObjectInitializer &ObjectInititalizer)
    : Super(ObjectInititalizer)
{}

void UItemWeaponRanged::Tick(float DeltaSeconds, const ACharacterCreature* Owner)
{
    check(Owner);
    UpdateSpread(DeltaSeconds);
    UpdateMultipliers(DeltaSeconds, Owner);
}

void UItemWeaponRanged::AddSpread()
{
    LastAddSpreadTime = GetWorld()->GetTimeSeconds();
    CurrentHeat = ClampHeat(CurrentHeat + HeatAddDueToInstabilityBase);
    CurrentSpreadAngle = HeatToSpreadMultiplier * CurrentHeat;
}

void UItemWeaponRanged::UpdateSpread(float DeltaSeconds)
{
    const float TimeSinceFired = GetWorld()->TimeSince(LastAddSpreadTime);

    if (TimeSinceFired > SpreadRecoveryCooldownDelay)
    {
        CurrentHeat = ClampHeat(CurrentHeat - (HeatCooldownRate * DeltaSeconds));
        CurrentSpreadAngle = HeatToSpreadMultiplier * CurrentHeat;
    }
}

void UItemWeaponRanged::UpdateMultipliers(float DeltaSeconds, const ACharacterCreature* Owner)
{
    const float MultiplierNearlyEqualThreshold = 0.05f;
    check(Owner != nullptr);
    UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(Owner->GetMovementComponent());

    // See if we are standing still, and if so, smoothly apply the bonus
    const float PawnSpeed = Owner->GetVelocity().Size();
    const float MovementTargetValue = FMath::GetMappedRangeValueClamped(
        /*InputRange=*/ FVector2D(StandingStillSpeedThreshold, StandingStillSpeedThreshold + StandingStillToMovingSpeedRange),
        /*OutputRange=*/ FVector2D(SpreadAngleMultiplier_StandingStill, SpreadAngleMultiplier_Running),
        /*Alpha=*/ PawnSpeed);
    StandingStillMultiplier = FMath::FInterpTo(StandingStillMultiplier, MovementTargetValue, DeltaSeconds, TransitionRate_StandingStill);

    // See if we are crouching, and if so, smoothly apply the bonus
    const bool bIsCrouching = (CharMovementComp != nullptr) && CharMovementComp->IsCrouching();
    const float CrouchingTargetValue = bIsCrouching ? SpreadAngleMultiplier_Crouching : 1.0f;
    CrouchingMultiplier = FMath::FInterpTo(CrouchingMultiplier, CrouchingTargetValue, DeltaSeconds, TransitionRate_Crouching);

    // See if we are in the air (jumping/falling), and if so, smoothly apply the penalty
    const bool bIsJumpingOrFalling = (CharMovementComp != nullptr) && CharMovementComp->IsFalling();
    const float JumpFallTargetValue = bIsJumpingOrFalling ? SpreadAngleMultiplier_JumpingOrFalling : 1.0f;
    JumpFallMultiplier = FMath::FInterpTo(JumpFallMultiplier, JumpFallTargetValue, DeltaSeconds, TransitionRate_JumpingOrFalling);

    // Determine if we are aiming down sights, and apply the bonus based on how far into the camera transition we are
    const float AimingMultiplier = 1.f;  // TODO implement aiminig?

    // Combine all the multipliers
    const float CombinedMultiplier = AimingMultiplier * StandingStillMultiplier * CrouchingMultiplier * JumpFallMultiplier;
    CurrentSpreadAngleMultiplier = CombinedMultiplier;
}

UItemShield::UItemShield(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{
    WeaponType = EWeaponType::Shield;
}
