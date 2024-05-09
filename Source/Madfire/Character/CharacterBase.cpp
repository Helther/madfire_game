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


    // Collision
    GetCapsuleComponent()->SetGenerateOverlapEvents(true);
    GetMesh()->SetGenerateOverlapEvents(false);

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
    AttributeComponent->SetUpdate(true);
    Update(FCharacterLevelInfo());
}

void ACharacterBase::DropItem(UItem *item, int32 amount, FTransform transform)
{
    AItemActor *actor = Cast<AItemActor>(GetWorld()->SpawnActor(item->ItemActorInWorld, &transform));
    SetupItemActorCollision(actor, true);
    actor->ItemBase = item;
    actor->StackCount = amount;
}

int32 ACharacterBase::AddMaterial()
{
    return 0;
}

void ACharacterBase::SetupItemActorCollision(AItemActor* InActor, bool Enable)
{
    if (InActor->ItemMesh)
    {
        InActor->ItemMesh->SetSimulatePhysics(Enable);
        InActor->ItemMesh->SetCollisionEnabled(Enable ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::QueryOnly);
        InActor->ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, Enable ? ECollisionResponse::ECR_Block : ECollisionResponse::ECR_Ignore);
        InActor->ItemCollision->SetCollisionResponseToChannel(EInteractChannel, Enable ? ECollisionResponse::ECR_Block : ECollisionResponse::ECR_Ignore);
    }
    if (!Enable)
        InActor->ItemCollision->SetGenerateOverlapEvents(false);
}

FVector ACharacterBase::GetProjectileSpawnLocation(ESpawnPivot Pivot,
                                                   FVector Offset) const
{
    Offset = GetActorRotation().RotateVector(Offset);
    switch (Pivot)
    {
        case ESpawnPivot::Root:
        default:  
            return GetMesh()->GetSocketLocation("Root") + Offset;
    }
}
