#include "CharacterBase.h"
#include "Components/PawnNoiseEmitterComponent.h"

ACharacterBase::ACharacterBase(const FObjectInitializer &ObjectInitializer) : 
    Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    AttributeComponent = CreateDefaultSubobject<UAttributeComponent>("AttributeComponent");
    EffectComponent = CreateDefaultSubobject<UEffectComponent>("EffectComponent");
    NoiseEmitterComponent = CreateDefaultSubobject<UPawnNoiseEmitterComponent>("NoiseEmitterComponent");
    Cheats.Invulnerable = false;
    Cheats.InvulnerableToHit = false;

    Logger.RegisterSubLogger(AttributeComponent, true);
    Logger.RegisterSubLogger(EffectComponent, true);
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


