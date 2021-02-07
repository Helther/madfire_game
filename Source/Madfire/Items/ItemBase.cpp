// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase.h"


UItemBase::UItemBase(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{}

void UItemBase::TakeDamage(float, const struct FDamageEvent &,
                           class AActor *EventInstigator)
{}

bool UItemBase::OnInteract_Implementation(EInteractType EInteractType,
                                          ACharacter *controller)
{
    return true;
}

uint8 UItemBase::GetInteractableFlags_Implementation() const { return InteractFlags; }

bool UItemBase::OnBeginFocus_Implementation() { return true; }

bool UItemBase::OnEndFocus_Implementation() { return true; }


// Sets default values
AItemActor::AItemActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Item Mesh"));

    RootComponent = ItemMesh;
}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
    Super::BeginPlay();

    if (ItemBase)
        if (ItemBase->ItemMesh)
            ItemMesh->SetSkeletalMesh(ItemBase->ItemMesh);
}

void AItemActor::OnConstruction(const FTransform &Transform)
{
    Super::OnConstruction(Transform);

    if (ItemBase)
        if (ItemBase->ItemMesh)
            ItemMesh->SetSkeletalMesh(ItemBase->ItemMesh);
}

bool AItemActor::OnInteract_Implementation(EInteractType EInteractType,
                                           ACharacter *controller)
{
    return true;
}

uint8 AItemActor::GetInteractableFlags_Implementation() const
{
    if (IsValid(ItemBase))
        return ItemBase->GetInteractableFlags_Implementation()
          | (uint8)EInteractType::Pickup;

    return 0;
}

bool AItemActor::OnBeginFocus_Implementation() { return true; }

bool AItemActor::OnEndFocus_Implementation() { return true; }

// Called every frame
void AItemActor::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
