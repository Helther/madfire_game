#include "CharacterBase.h"
#include "Components/BoxComponent.h"
#include "TraceFunctional/TraceDefines.h"
#include "Components/PawnNoiseEmitterComponent.h"

ACharacterBase::ACharacterBase(const FObjectInitializer &ObjectInitializer) : 
    Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    AttributeComponent = CreateDefaultSubobject<UAttributeComponent>("AttributeComponent");
    StatusEffectComponent = CreateDefaultSubobject<UStatusEffectQueueComponent>("StatusEffectComponent");
    NoiseEmitterComponent = CreateDefaultSubobject<UPawnNoiseEmitterComponent>("NoiseEmitterComponent");
    Cheats.Invulnerable = false;
    Cheats.InvulnerableToHit = false;

    Logger.RegisterSubLogger(AttributeComponent, true);
    Logger.RegisterSubLogger(StatusEffectComponent, true);
}

float ACharacterBase::TakeDamage(float DamageAmount, const struct FDamageEvent &DamageEvent,
                                 class AController *EventInstigator,
                                 AActor *DamageCauser)
{
    const auto DamageType = DamageEvent.DamageTypeClass.GetDefaultObject();
    auto RemainingHealth = AttributeComponent->TakeDamage(DamageAmount, DamageType, DamageCauser);
    if(RemainingHealth <= 0)
    {
        //TODO: Set Dead State
        //TODO: Or Dead Event;
    }

    return RemainingHealth;
}

void ACharacterBase::Update(const FCharacterLevelInfo &levelInfo)
{
    if (AttributeComponent->IsUpdateNeeded())
        AttributeComponent->Update();
    // TODO apply bonuses from EffectComponent
}

void ACharacterBase::BeginPlay()
{
    Super::BeginPlay();

    Logger.Init(this);
    // init components
    AttributeComponent->SetDirty();
    Update(FCharacterLevelInfo());
}

void ACharacterBase::DropItem(UItem *item, int32 amount, FTransform transform)
{
    AItemActor *actor = Cast<AItemActor>(GetWorld()->SpawnActor(item->ItemActor, &transform));
    actor->ItemMesh->SetSimulatePhysics(true);
    actor->ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    actor->ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
    actor->ItemCollision->SetCollisionResponseToChannel(EInteractChannel, ECollisionResponse::ECR_Block);
    actor->ItemBase = DuplicateObject(item,actor->GetOuter());
    actor->StackCount = amount;
}
