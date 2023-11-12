// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase.h"
#include "Components/BoxComponent.h"
#include "../TraceFunctional/TraceDefines.h"
#include "Character/CharacterCreature.h"
#include "Utils/UtilFunctions.h"

const FName FItemActorEquipmentSlotNames::WeaponLoadedAmmo = "S_WeaponLoadedAmmo";
const FName FItemActorEquipmentSlotNames::EquipSocket = "S_EquipSocket";
const FName FItemActorEquipmentSlotNames::WeaponEquipSocket = "S_WeaponEquipSocket";

UItem::UItem(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{
    ItemInteractFlags = 1 << (int32)EInteractType::Pickup;
}

bool UItem::OnInteract_Implementation(EInteractType InteractType, ACharacter *controller)
{
    return true;
}

int32 UItem::GetInteractFlags_Implementation() const { return ItemInteractFlags; }
FText UItem::GetInteractableName_Implementation() const { return ItemName; }

bool UItem::OnBeginFocus_Implementation() { return true; }

bool UItem::OnEndFocus_Implementation() { return true; }


AItemActor::AItemActor()
{
    InitializeActor();
}

AItemActor::AItemActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InitializeActor();
}

float AItemActor::TakeDamage(float DamageAmount, const FDamageEvent &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
    check(ItemBase != nullptr && "AItemActor: Item Base is invalid");
    ItemBase->TakeDamage(DamageAmount, DamageEvent, DamageCauser);
    return 0.f;
}

void AItemActor::InitializeActor()
{
    PrimaryActorTick.bCanEverTick = true;

    ItemCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Item Collision"));
    ItemCollision->SetCollisionResponseToChannel(EWeaponChannel, ECollisionResponse::ECR_Ignore);
    ItemCollision->SetCollisionResponseToChannel(EInteractChannel, ECollisionResponse::ECR_Block);
    ItemCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    ItemCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
    ItemCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    
    StackCount = 1;
}

void AItemActor::InitMesh()
{
    ItemMesh->SetGenerateOverlapEvents(false);
    ItemMesh->SetSimulatePhysics(true);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ItemMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);

    SetRootComponent(ItemMesh);
    ItemCollision->SetupAttachment(RootComponent);
}

bool AItemActor::OnInteract_Implementation(EInteractType InteractType, ACharacter *controller)
{
    if(int(InteractType & EInteractType::Pickup) != 0)
    {
        auto Character = Cast<ACharacterCreature>(controller);
        if(Character)
        {
            Character->PickupItem(this);
        }
        return false;
    }
    else
    {
        return ItemBase->OnInteract(InteractType, controller);
    }
}

int32 AItemActor::GetInteractFlags_Implementation() const
{
    check(ItemBase != nullptr && "AItemActor: Item Base is invalid");
    return ItemBase->ItemInteractFlags;
}

FText AItemActor::GetInteractableName_Implementation() const
{
    check(ItemBase != nullptr && "AItemActor: Item Base is invalid");
    return ItemBase->GetInteractableName();
}

bool AItemActor::OnBeginFocus_Implementation() { return true; }

bool AItemActor::OnEndFocus_Implementation() { return true; }


AItemActorEquipment::AItemActorEquipment()
{
    InitSkeletalMesh();
}

AItemActorEquipment::AItemActorEquipment(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InitSkeletalMesh();
}

void AItemActorEquipment::InitSkeletalMesh()
{
    ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Item Mesh"));
    InitMesh();
}

AItemActorWorld::AItemActorWorld()
{
    InitStaticMesh();
}

AItemActorWorld::AItemActorWorld(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InitStaticMesh();
}

void AItemActorWorld::InitStaticMesh()
{
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item Mesh"));
    InitMesh();
}

AItemActorProjectile::AItemActorProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    LifeTime = 30;
    MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComp"));

    MovementComp->bShouldBounce = false;
    MovementComp->bSimulationEnabled = false;
    MovementComp->ProjectileGravityScale = 0.0f;
    MovementComp->OnProjectileStop.AddUniqueDynamic(this, &AItemActorProjectile::OnStop);

    ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
    ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

    ItemCollision->SetCollisionResponseToChannel(EInteractChannel, ECollisionResponse::ECR_Ignore);
    ItemCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    ItemCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
    ItemCollision->SetGenerateOverlapEvents(false);

    ItemCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &AItemActorProjectile::OnOverlap);
}

void AItemActorProjectile::LaunchProjectile(FVector Direction, float Speed, float DamageCoefficient)
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    MovementComp->SetUpdatedComponent(ItemMesh);
    MovementComp->bSimulationEnabled = true;
    auto ownerCharacter = Cast<ACharacter>(Owner);
    auto ammoItem = Cast<UItemAmmo>(ItemBase);
    MovementComp->ProjectileGravityScale = ammoItem->WeightCoefficient;
    MovementComp->bRotationFollowsVelocity = true;
    InitialSpeed = Speed;
    WeaponDamageCoefficient = DamageCoefficient;
    MovementComp->Velocity = Direction * Speed;
    ItemCollision->SetGenerateOverlapEvents(true);
}

void AItemActorProjectile::OnOverlap(UPrimitiveComponent* comp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // check if hit not self, check if hit actor call take damage, attach ammo to actor
    // if hit shield stop, if character - check his physical material in hit result and deduce body part - apply effect for body part
    auto hitActor = SweepResult.GetActor();
    auto itemActor = Cast<AItemActor>(hitActor);
    bool hitSelfWeapon = (itemActor && itemActor->ItemCollision == comp) ||(itemActor && itemActor->ItemMesh == comp);
    if (hitActor == nullptr || OtherActor == Owner || hitSelfWeapon)// Check self and weapon
        return;

    MovementComp->StopSimulating(SweepResult);
    USceneComponent* attachMesh = nullptr;
    if (itemActor)
        attachMesh = itemActor->ItemMesh;
    else if (auto character = Cast<ACharacter>(hitActor))
        attachMesh = character->GetMesh();
    if (attachMesh)
        AttachToHitActor(attachMesh, GetActorLocation());

    auto shield = itemActor ? Cast<UItemShield>(itemActor->ItemBase) : nullptr;
    if (shield)
    {
        UE_LOG(LogTemp, Warning, TEXT("AItemActorProjectile::OnOverlap: hit a shield"));
        return;
    }

    auto ownerCharacter = Cast<ACharacterCreature>(Owner);
    auto ammoItem = Cast<UItemAmmo>(ItemBase);
    float damage = ammoItem->DamageValue + InitialSpeed * WeaponDamageCoefficient;
    float force = ammoItem->WeightCoefficient * ammoItem->ForceCoefficient;
    hitActor->TakeDamage(damage, FForceDamageEvent(ammoItem->DamageType, force), ownerCharacter->GetController(), Owner);
    //if (attachMesh)
    //    SetOwner(hitActor);  // if attached to actor switch ownership to it, so arrow will auto destroy with it TODO doesn't work
}

void AItemActorProjectile::OnStop(const FHitResult& SweepResult)
{
    ItemCollision->SetGenerateOverlapEvents(true);
    ItemCollision->SetCollisionResponseToChannel(EInteractChannel, ECollisionResponse::ECR_Block);
    ItemCollision->OnComponentBeginOverlap.RemoveDynamic(this, &AItemActorProjectile::OnOverlap);
    
    GetWorld()->GetTimerManager().SetTimer(DestroyTimer, this, &AItemActorProjectile::OnTimeout, LifeTime);
}

void AItemActorProjectile::OnTimeout()
{
    Destroy();
}

void AItemActorProjectile::AttachToHitActor(USceneComponent* ActorMesh, FVector HitLocation)
{
    FName AttachSocket = MadFireUtils::GetNearestSocketToLocation(ActorMesh, HitLocation);
    if (!AttachSocket.IsNone())
    {
        FTransform socketTransform = ActorMesh->GetSocketTransform(AttachSocket);
        FVector relativeLocation = socketTransform.InverseTransformPosition(GetActorLocation());
        FQuat relativeRot = socketTransform.InverseTransformRotation(GetActorQuat());
        SetActorRelativeLocation(relativeLocation);
        SetActorRelativeRotation(relativeRot);
    }
    else
    {
        SetActorRelativeLocation(FVector::ZeroVector);
        SetActorRelativeRotation(FQuat::Identity);
    }

    FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, false);
    AttachToComponent(ActorMesh, rules, AttachSocket);
}

AItemActorEquipmentMerge::AItemActorEquipmentMerge()
{
    SetRootComponent(ItemCollision);
}

AItemActorEquipmentMerge::AItemActorEquipmentMerge(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetRootComponent(ItemCollision);
}
