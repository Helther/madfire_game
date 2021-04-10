#include "CharacterBase.h"

#include "Components/CharacterAnimInstance.h"


ACharacterBase::ACharacterBase(const FObjectInitializer &ObjectInitializer) :
    Super(ObjectInitializer)
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    AttributesComponent = CreateDefaultSubobject<UAttributesComponent>("AttributesComponent");
    ExperienceComponent  = CreateDefaultSubobject<UExperienceComponent>("ExperienceComponent");
    EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>("EquipmentComponent");
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");
    EffectComponent = CreateDefaultSubobject<UEffectComponent>("EffectComponent");
    MaxWeightMod   = 10.f;
}

float ACharacterBase::TakeDamage(float DamageAmount, const struct FDamageEvent &DamageEvent,
                                 class AController *EventInstigator,
                                 AActor *DamageCauser)
{
    const auto DamageType = DamageEvent.DamageTypeClass.GetDefaultObject();

    const auto PhysD  = Cast<UPhysicDamage>(DamageType);
    const auto MagicD = Cast<UMagicDamage>(DamageType);

    if (PhysD != nullptr)
    {
        auto Type  = PhysD->Type;
        auto Force = PhysD->Force;
        DamageAmount -= AttributesComponent->CurrentDamageAbsorb.DamageMap[(int32)Type];
        DamageAmount = DamageAmount * (AttributesComponent->CurrentDamageResistance.DamageMap[(int32)Type] - 100) / 100;
        AttributesComponent->Health->Consume(DamageAmount); // -> bool
        //TODO: Death, Force, DamageItem
        return DamageAmount;
    }
    else if (MagicD != nullptr)
    { // тоже что и физический + проверка на пробитие магического щита если такой имеется
        auto Type        = MagicD->Type;
        auto MagicSchool = MagicD->MagicSchool;
        DamageAmount -= AttributesComponent->CurrentDamageAbsorb.DamageMap[(int32)Type];
        DamageAmount = DamageAmount * (AttributesComponent->CurrentDamageResistance.DamageMap[(int32)Type] - 100) / 100;
        if (DamageAmount < 0)
        {
            if (AttributesComponent->Health->Consume(DamageAmount))
                return DamageAmount = 0;
            else
                return DamageAmount - AttributesComponent->MagicShield->CurrentVal;
        }
        return DamageAmount = 0;
    }
    else if (DamageType) //if damage nor Physical or Magical, then itВґs raw Damage
    {
        AttributesComponent->Health->Consume(-DamageAmount); // -> bool
        return DamageAmount;
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
    if (ExperienceComponent->IsUpdateNeeded())
        ExperienceComponent->Update();
    if (AttributesComponent->IsUpdateNeeded())
    {
        AttributesComponent->Update();
        CalculateMaxWeight();
    }
    if (EquipmentComponent->IsUpdateNeeded())
        EquipmentComponent->Update();
    if (InventoryComponent->IsUpdateNeeded())
        InventoryComponent->Update();

    //other calculation
}

void ACharacterBase::DoLevelUp(const FLevelInfo &levelInfo)
{
    ExperienceComponent->LevelUp(levelInfo);
    ExperienceComponent->SetUpdate();
    AttributesComponent->SetUpdate();
    bool dexChanged = UAttributesHelper::GetAttribute(levelInfo.Attributes, EAttributes::Dex) != 0;
    bool strChanged = UAttributesHelper::GetAttribute(levelInfo.Attributes, EAttributes::Str) != 0;
    bool acrobaticsChanged = USkillHelper::GetValue(levelInfo.SkillList, ESkill::Tumble) != 0;

    if (strChanged)
    {
        InventoryComponent->SetUpdate();
        EquipmentComponent->SetUpdate();
    }
    Update();
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}


void ACharacterBase::CalculateMaxWeight()
{
    const int32 str = UAttributesHelper::GetAttribute(AttributesComponent->CurrentAttributes, EAttributes::Str);
    MaxWeight = MaxWeightMod * str; // clever formula!
}

