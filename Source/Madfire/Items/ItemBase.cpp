// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase.h"
#include "Components/BoxComponent.h"
#include "../TraceFunctional/TraceDefines.h"
#include "Character/CharacterCreature.h"

UItem::UItem(const class FObjectInitializer &ObjectInititalizer)
  : Super(ObjectInititalizer)
{}

bool UItem::OnInteract_Implementation(EInteractType InteractType, ACharacter *controller)
{
    return true;
}

uint8 UItem::GetInteractableFlags_Implementation() const { return InteractFlags; }
FText UItem::GetInteractableName_Implementation() const { return ItemName; }

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

float AItemActor::TakeDamage(float DamageAmount, const FDamageEvent &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
    check(ItemBase != nullptr && "AItemActor: Item Base is invalid");
    ItemBase->TakeDamage(DamageAmount,DamageEvent,DamageCauser);
    return 0.f;
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
        return ItemBase->OnInteract(InteractType,controller);
    }
}

uint8 AItemActor::GetInteractableFlags_Implementation() const
{
    check(ItemBase != nullptr && "AItemActor: Item Base is invalid");
    return ItemBase->GetInteractableFlags() | (uint8)EInteractType::Pickup;
}

FText AItemActor::GetInteractableName_Implementation() const
{
    check(ItemBase != nullptr && "AItemActor: Item Base is invalid");
    return ItemBase->GetInteractableName();
}

bool AItemActor::OnBeginFocus_Implementation() { return true; }

bool AItemActor::OnEndFocus_Implementation() { return true; }

// Called every frame
void AItemActor::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
