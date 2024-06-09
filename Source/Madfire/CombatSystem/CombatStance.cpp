#include "CombatStance.h"
#include "Character/CharacterCreature.h"
#include "Items/RangedWeaponAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"


//TODO: Better name needed

const FAttackInfo &UCombatStance::GetCurrentAttackInfo(ESpecialType type) const
{
    static FAttackInfo a;
    return a;
}

bool UCombatStance::IsLoaded() const 
{
    return false; 
}

bool UCombatStance::HasChargeAttack() const 
{
    return false; 
}

bool UCombatStance::CanDoShit(ESpecialType type) const 
{
    return false; 
}

bool  UCombatStance::IsMelee() const
{
    return true;
}

bool UCombatStance::CanDoShit(ECombatMovementType type) const
{
    return CombatMoves[StaticCast<unsigned long int>(type)] != nullptr;
}

bool UCombatStance::CanDoShit(ECombatMotion type) const
{
    return CombatMotions[StaticCast<unsigned long int>(type)] != nullptr;
}

void UCombatStance::StanceSet(ACharacterCreature *NewOwner)
{
    Owner = NewOwner;
    OnStanceActivated();
    //if (!Owner->GetCombatComponent()->WeaponDrawn())
        //ArmsSetup(false); // attach the weapons
}

void UCombatStance::StanceUnSet()
{    
    OnStanceDeActivated();
    //ResetWeaponVars();
    Owner = nullptr;
}

// check that weapons setup correctly

bool UCombatStance::IsStanceValid(const ACharacterCreature *NewOwner) const
{
    return true;
}

const FAttackInfo &UMeleeCombatStance::GetCurrentAttackInfo(ESpecialType type) const
{
    check(ComboAttacks.Num() != 0);
    if (type != ESpecialType::None)
        return *SpecialAttacks.FindByPredicate([type](const FAttackInfo &info) { return info.Special == type; });

    if (ComboAttacks.Num() != 0)
        return ComboAttacks[0].NormalAttack;

    return UCombatStance::GetCurrentAttackInfo(type);
}

inline bool UMeleeCombatStance::CanDoShit(ESpecialType type) const
{
    if (type != ESpecialType::None)
        return SpecialAttacks.FindByPredicate([type](const FAttackInfo &info) {
            return info.Special == type;
        }) != nullptr;
    else
        return true;
}

bool UMeleeCombatStance::HasChargeAttack() const
{
    return ComboAttacks[0].ChargeAttackStages.Num() > 0;
}

UMeleeCombatStance::UMeleeCombatStance() : 
    UCombatStance()
{}

bool UMeleeCombatStance::IsStanceValid(const ACharacterCreature* NewOwner) const
{    
    if (IsUsingDefaultWeapons && (NewOwner->GetEquipmentComponent()->GetEquipmentFromSlot(EItemSlot::MainWeaponSlot).Item || 
                                  NewOwner->GetEquipmentComponent()->GetEquipmentFromSlot(EItemSlot::OffMeaponSlot).Item))
        return false; // don't set unarmed stance if armed

    //auto mainWeapon = GetArmsFromSlot(EArmsSlot::MainHand);
    auto mainWeapon = NewOwner->GetCombatComponent()->GetArmItem(EArmsSlot::MainHand);
    //auto offWeapon = GetArmsFromSlot(EArmsSlot::OffHand); 
    //auto shield = GetArmsFromSlot(EArmsSlot::Shield); 
    auto offWeapon = NewOwner->GetCombatComponent()->GetArmItem(EArmsSlot::OffHand); 
    auto shield = NewOwner->GetCombatComponent()->GetArmItem(EArmsSlot::Shield); 

    switch (StanceType)
    {
    case EWeaponCombination::TwoWeapon:
        return mainWeapon && AllowedWeaponTypes.Contains(mainWeapon->WeaponType) && 
            offWeapon && AllowedWeaponTypes.Contains(offWeapon->WeaponType);
    case EWeaponCombination::TwoHand:
    case EWeaponCombination::OneHand:
    {
        bool hasOffhandEquipped = IsUsingDefaultWeapons ? false : (NewOwner->GetEquipmentComponent()->GetEquipmentFromSlot(EItemSlot::OffMeaponSlot).Item != nullptr);
        return mainWeapon && AllowedWeaponTypes.Contains(mainWeapon->WeaponType) && !offWeapon && !hasOffhandEquipped;
    }
    case EWeaponCombination::Shield:
        return mainWeapon && AllowedWeaponTypes.Contains(mainWeapon->WeaponType) && 
            shield && (shield->WeaponType == EWeaponType::Shield) && AllowedWeaponTypes.Contains(shield->WeaponType);
    default:
        return false;
    }
}

bool UMeleeCombatStance::IsMelee() const 
{
    return true; 
}


URangeCombatStance::URangeCombatStance() :
    UCombatStance(),
    AnimRateMod(EAttributes::Dex)
{}

bool URangeCombatStance::IsStanceValid(const ACharacterCreature *NewOwner) const
{
    //auto mainWeapon = GetArmsFromSlot(EArmsSlot::MainHand);
    //auto offWeapon = GetArmsFromSlot(EArmsSlot::OffHand); 
    auto mainWeapon = NewOwner->GetCombatComponent()->GetArmItem(EArmsSlot::MainHand);
    auto offWeapon = NewOwner->GetCombatComponent()->GetArmItem(EArmsSlot::OffHand); 

    return !offWeapon && mainWeapon && StanceType == EWeaponCombination::Ranged && AllowedWeaponTypes.Contains(mainWeapon->WeaponType);
}

float URangeCombatStance::CalculateAmmoInitialSpeed(UItemWeaponRanged* Weapon) const
{
    return Weapon->MaxTensionStrength * Weapon->TensionCoefficient;
}

void URangeCombatStance::StanceSet(ACharacterCreature* NewOwner)
{
    UCombatStance::StanceSet(NewOwner);
}

void URangeCombatStance::StanceUnSet()
{
    CleanUpAmmoActor();
    UCombatStance::StanceUnSet();
}

FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleRad)
{
    if (ConeHalfAngleRad > 0.f)
    {
        const float ConeHalfAngleDegrees = FMath::RadiansToDegrees(ConeHalfAngleRad);

        // consider the cone a concatenation of two rotations. one "away" from the center line, and another "around" the circle
        // apply the exponent to the away-from-center rotation. a larger exponent will cluster points more tightly around the center
        const float FromCenter = FMath::FRand();
        const float AngleFromCenter = FromCenter * ConeHalfAngleDegrees;
        const float AngleAround = FMath::FRand() * 360.0f;

        FRotator Rot = Dir.Rotation();
        FQuat DirQuat(Rot);
        FQuat FromCenterQuat(FRotator(0.0f, AngleFromCenter, 0.0f));
        FQuat AroundQuat(FRotator(0.0f, 0.0, AngleAround));
        FQuat FinalDirectionQuat = DirQuat * AroundQuat * FromCenterQuat;
        FinalDirectionQuat.Normalize();

        return FinalDirectionQuat.RotateVector(FVector::ForwardVector);
    }
    else
        return Dir.GetSafeNormal();
}

void URangeCombatStance::FireLoadedAmmo()
{
    check(LoadedAmmo);
    IsWeaponLoaded = false;

    FVector CameraLocation;
    FRotator CameraRotation;
    AController* Controller = Owner->GetController();
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (PC != nullptr)
        PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
    else
    {
        CameraLocation = Owner->GetActorLocation();
        CameraRotation = Controller->GetControlRotation();
    }
    //AItemActor* weaponActor = Owner->GetCombatComponent()->GetCurrentCombatStance()->GetArmsActorFromSlot(EArmsSlot::MainHand);
    AItemActor* weaponActor = Owner->GetCombatComponent()->GetArmActor(EArmsSlot::MainHand);
    auto rangedWeapon = Cast<UItemWeaponRanged>(weaponActor->ItemBase);

    const float BaseSpreadAngle = rangedWeapon->GetCalculatedSpreadAngle();
    const float SpreadAngleMultiplier = rangedWeapon->GetCalculatedSpreadAngleMultiplier();
    const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;
    const float HalfSpreadAngleInRadians = FMath::DegreesToRadians(ActualSpreadAngle * 0.5f);

    FVector MuzzleDirection = CameraRotation.Vector().GetSafeNormal();
    MuzzleDirection = VRandConeNormalDistribution(MuzzleDirection, HalfSpreadAngleInRadians);  // apply spread
    FVector AmmoLocation = LoadedAmmo->GetActorLocation();
    float MaxFocalDistance = 2048.f;  // TODO devise or calculate
    float MinFocalDistance = 128.f;
    
    FTraceParams traceParams;
    traceParams.isDebug = false;
    traceParams.ActorsToIgnore.Add(Owner);
    // TODO trace endpoint location unfortunately depends of camera zoom distance
    FTraceSpawnParams traceSpawnParams;
    traceSpawnParams.Start = CameraLocation;
    traceSpawnParams.End = CameraLocation + (MuzzleDirection * MaxFocalDistance);

    FVector LaunchDirection;
    FHitResult hit(ForceInit);
    // Calculate the direction vector, between two points
    if(UTraceManager::LineTraceSingleByChannel(GetWorld(), traceParams, traceSpawnParams, hit, ECollisionChannel::ECC_Visibility))
        LaunchDirection = hit.ImpactPoint - AmmoLocation;
    else
        LaunchDirection = traceSpawnParams.End - AmmoLocation;
    if (LaunchDirection.Length() < MinFocalDistance)  // if hit was too close to the pawn use MinFocalDistance
        LaunchDirection = CameraLocation + (MuzzleDirection * MaxFocalDistance) - AmmoLocation;
    LaunchDirection.Normalize();
    LaunchDirection = LaunchDirection.Rotation().Vector();

    float LaunchInitialSpeed = CalculateAmmoInitialSpeed(rangedWeapon);
    LoadedAmmo->StackCount = 1;
    LoadedAmmo->LaunchProjectile(LaunchDirection, LaunchInitialSpeed, rangedWeapon->AmmoDamageCoefficient);
    LoadedAmmo = nullptr;
    // TODO LoadAmmoCount > 1 don't deload, spawn another ammo actor instead
    if (auto actorMesh = Cast<USkeletalMeshComponent>(weaponActor->ItemMesh))
        if (auto animInstance = Cast<URangedWeaponAnimInstance>(actorMesh->GetAnimInstance()))
            animInstance->IsFired = true;
}

void URangeCombatStance::SpawnAttachAmmoActor()
{
    auto baseActor = Owner->GetEquipmentComponent()->GetEquipmentFromSlot(EItemSlot::AmmoSlot).Item;
    auto baseItem = baseActor->ItemBase;
    auto ammoItem = Cast<UItemAmmo>(baseItem);
    auto newAmmoActor = GetWorld()->SpawnActor<AItemActorProjectile>(ammoItem->ProjectileActor);
    auto newAmmoItem = DuplicateObject(baseItem, newAmmoActor);

    ACharacterBase::SetupItemActorCollision(newAmmoActor, false);
    newAmmoActor->SetOwner(Owner);
    newAmmoActor->ItemBase = newAmmoItem;
    newAmmoActor->StackCount = LoadAmmoCount;
    if (baseActor->StackCount > LoadAmmoCount)
        baseActor->StackCount -= LoadAmmoCount;
    else
        Owner->GetEquipmentComponent()->Unequip(EItemSlot::AmmoSlot);

    // attach to character left grip hand slot
    newAmmoActor->AttachToComponent(Owner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FEquipmentSlotNames::WeaponOff);
    LoadedAmmo = newAmmoActor;
}

void URangeCombatStance::AttachAmmoActorToWeapon()
{
    check(LoadedAmmo);
    //AItemActor* weaponActor = GetArmsActorFromSlot(EArmsSlot::MainHand);
    AItemActor* weaponActor = Owner->GetCombatComponent()->GetArmActor(EArmsSlot::MainHand);

    check(weaponActor);
    LoadedAmmo->AttachToComponent(weaponActor->ItemMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FItemActorEquipmentSlotNames::WeaponLoadedAmmo);
}

inline const FAttackInfo &URangeCombatStance::GetCurrentAttackInfo(ESpecialType type) const
{
    return AttackInfo;
}

void URangeCombatStance::CleanUpAmmoActor()
{
    IsWeaponLoaded = false;
    if (LoadedAmmo)
    {
        auto ammoItem = LoadedAmmo->ItemBase;
        Owner->GetInventoryComponent()->AddToInventory(ammoItem, LoadAmmoCount); // return loaded ammo to the inventory TODO until we think of something better
        LoadedAmmo->Destroy();
        LoadedAmmo = nullptr;
    }
}

bool URangeCombatStance::CanDoShit(ESpecialType type) const
{
    return IsWeaponLoaded && AttackInfo.AttackMontage != nullptr;
}

UBowCombatStance::UBowCombatStance() : 
    URangeCombatStance(),
    ChargeStaminaConsumeDelta(0.5f),
    LastChargeTimeStaminaConsume(0),
    LastChargeTime(0)
{}

float UBowCombatStance::GetDrawRatio() const
{
    return Owner->GetCombatComponent()->CombatFSM.currentState()->ChargeTime / GetMaxDrawTime();
}

float UBowCombatStance::CalculateAmmoInitialSpeed(UItemWeaponRanged* Weapon) const
{
    return Weapon->MaxTensionStrength * Weapon->TensionCoefficient * GetLastDrawRatio();
}

float UBowCombatStance::GetMaxDrawTime() const
{
    //AItemActor* weaponActor = Owner->GetCombatComponent()->GetCurrentCombatStance()->GetArmsActorFromSlot(EArmsSlot::MainHand);
    AItemActor* weaponActor = Owner->GetCombatComponent()->GetArmActor(EArmsSlot::MainHand);
    check(weaponActor);
    auto weaponItem = Cast<UItemWeaponRanged>(weaponActor->ItemBase);
    check(weaponItem);
    EAttributes modAttribute = weaponItem->WeaponType == EWeaponType::Longbow ? EAttributes::Str : EAttributes::Dex;
    check(BowDrawingAnim);
    float animDuration = BowDrawingAnim->GetPlayLength();
    float animRateModifier = Owner->GetAttributeComponent()->GetAnimRateFromModifier(modAttribute);
    float playRate = animDuration / (weaponItem->MaxTensionStrength / animRateModifier) * Owner->GetCombatComponent()->DrawTimeModFromTension;
    return animDuration / playRate;
}

float UBowCombatStance::GetLastDrawRatio() const
{
    return LastChargeTime / GetMaxDrawTime();
}

UCrossbowCombatStance::UCrossbowCombatStance() :
    URangeCombatStance()    
{
    AnimRateMod = EAttributes::Str;
}
