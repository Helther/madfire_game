// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase.h"
#include "Components/BoxComponent.h"
#include "../TraceFunctional/TraceDefines.h"

UItem::UItem(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{}

void UItem::TakeDamage(float, const struct FDamageEvent &, class AActor *EventInstigator)
{}

bool UItem::OnInteract_Implementation(EInteractType EInteractType, ACharacter *controller)
{
    return true;
}

uint8 UItem::GetInteractableFlags_Implementation() const { return InteractFlags; }

bool UItem::OnBeginFocus_Implementation() { return true; }

bool UItem::OnEndFocus_Implementation() { return true; }


// Sets default values
AItemActor::AItemActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item Mesh"));

    ItemCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Item Collision"));
    ItemMesh->SetCollisionProfileName("NoCollision");
    ItemMesh->SetGenerateOverlapEvents(false);
    SetRootComponent(ItemMesh);
    ItemCollision->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
    Super::BeginPlay();
}

void AItemActor::OnConstruction(const FTransform &Transform)
{
    Super::OnConstruction(Transform);
}

bool AItemActor::OnInteract_Implementation(EInteractType EInteractType, ACharacter *controller)
{
    return true;
}

uint8 AItemActor::GetInteractableFlags_Implementation() const
{
    if (IsValid(ItemBase))
        return ItemBase->GetInteractableFlags_Implementation() | (uint8)EInteractType::Pickup;
    return 0;
}

bool AItemActor::OnBeginFocus_Implementation() { return true; }

bool AItemActor::OnEndFocus_Implementation() { return true; }

// Called every frame
void AItemActor::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
