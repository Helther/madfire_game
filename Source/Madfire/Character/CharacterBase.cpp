#include "CharacterBase.h"
#include "CharacterAnimInstance.h"


ACharacterBase::ACharacterBase(const FObjectInitializer &ObjectInitializer) :
    Super(ObjectInitializer)
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    //AttributesComponent = CreateDefaultSubobject<UAttributesComponent>("AttributesComponent");
    AttributeComponent = CreateDefaultSubobject<UAttributeComponent>("AttributeComponent");
    //ExperienceComponent  = CreateDefaultSubobject<UExperienceComponent>("ExperienceComponent");
    EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>("EquipmentComponent");
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");
    EffectComponent = CreateDefaultSubobject<UEffectComponent>("EffectComponent");
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
    MaxWeightMod   = 10.f;
}

float ACharacterBase::TakeDamage(float DamageAmount, const struct FDamageEvent &DamageEvent,
                                 class AController *EventInstigator,
                                 AActor *DamageCauser)
{
    const auto DamageType = DamageEvent.DamageTypeClass.GetDefaultObject();
    //TODO: Combat Component check something

    auto RemountHealth = AttributeComponent->TakeDamage(DamageAmount,DamageType,DamageCauser);

    if(RemountHealth <= 0)
    {
        //TODO: Set Dead State
        //TODO: Or Dead Event;
    }

    return 0;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
    Super::BeginPlay();
}

void ACharacterBase::OnUpdate()
{
    this->Update();
}

void ACharacterBase::Update()
{
    //if (ExperienceComponent->IsUpdateNeeded())
        //ExperienceComponent->Update();
    if (AttributeComponent->IsUpdateNeeded())
    {
        AttributeComponent->Update();
        CalculateMaxWeight();
    }
    if (EquipmentComponent->IsUpdateNeeded())
        EquipmentComponent->Update();
    if (InventoryComponent->IsUpdateNeeded())
        InventoryComponent->Update();

    //other calculation
}


// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}


void ACharacterBase::CalculateMaxWeight()
{
    const int32 str = UAttributeSetHelper::GetAttribute(AttributeComponent->Attributes, EAttributes::Str);
    MaxWeight = MaxWeightMod * str; // clever formula!
}

