#include "CharacterBase.h"


ACharacterBase::ACharacterBase(const FObjectInitializer &ObjectInitializer) : 
    Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    AttributeComponent = CreateDefaultSubobject<UAttributeComponent>("AttributeComponent");
    EffectComponent = CreateDefaultSubobject<UEffectComponent>("EffectComponent");
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

    // init components
    AttributeComponent->SetDirty();
    Update(FCharacterLevelInfo());
}
