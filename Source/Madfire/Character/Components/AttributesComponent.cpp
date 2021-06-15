// Fill out your copyright notice in the Description page of Project Settings.

#include "AttributesComponent.h"
#include "Character/CharacterBase.h"


UAttributeComponent::UAttributeComponent(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    Health = ObjectInitializer.CreateDefaultSubobject<UDynamicAttribute>(this,"Health",false);
    Stamina = ObjectInitializer.CreateDefaultSubobject<UDynamicAttribute>(this,"Stamina",false);
    CharacterRace = ObjectInitializer.CreateDefaultSubobject<URaceInfo>(this,"Race",false);
}

void UAttributeComponent::AddExperience(float Amount)
{
    CurrentExperience += Amount;
}

float UAttributeComponent::GetLevelupRequiredExp() const
{
    verify(ExperienceCurve != nullptr && "Experience Curve is not Set");

    const auto NextLevel = LevelList.Num() + 1;
    auto NextLvlExp = ExperienceCurve->GetFloatValue(NextLevel);
    return FMath::RoundHalfFromZero(NextLvlExp);
}

void UAttributeComponent::LevelUp(const FCharacterLevelInfo &LevelInfo)
{
    check((LevelList.Num() <= MaxLevel) && "Level limit is 30");
    auto IntMod = UAttributeSetHelper::GetModifier(Attributes, EAttributes::Int);
    FreeAttributePoints += 1;
    FreeSkillPoints += 10+IntMod; //TODO: Number  To Variable
    LevelList.Add(LevelInfo);        
    Update();
}

void UAttributeComponent::UpdateBaseAttribute()
{
    BaseAttribute = CharacterRace->Attributes;
    for (auto &levelInfo : LevelList)
        BaseAttribute += levelInfo.Attributes;

}

void UAttributeComponent::UpdateAttribute()
{
    auto Character = Cast<ACharacterBase>(GetOwner());
    const UEquipmentComponent *Equipment = Character->EquipmentComponent;
    BaseAttribute = CharacterRace->Attributes;
    for (auto &levelInfo : LevelList)
        BaseAttribute += levelInfo.Attributes;

    Attributes = BaseAttribute + BonusAttribute + Equipment->EquipmentAttributes;
}

void UAttributeComponent::UpdateSkills()
{
    auto Character = Cast<ACharacterBase>(GetOwner());
    const UEquipmentComponent *Equipment = Character->EquipmentComponent;
    auto LevelSkills      = FSkillList();
    for (auto &levelInfo : LevelList)
        LevelSkills += levelInfo.SkillList;
    Skills = LevelSkills + BonusSkills + Equipment->EquipmentSkills;
}

void UAttributeComponent::UpdateDamageAbsorb()
{
    auto Character = Cast<ACharacterBase>(GetOwner());
    const UEquipmentComponent *Equipment = Character->EquipmentComponent;
    DamageAbsorb = BonusDamageAbsorb + Equipment->EqipmentDamageAbsorb;
}

void UAttributeComponent::UpdateDamageResist()
{
    auto Character = Cast<ACharacterBase>(GetOwner());
    const UEquipmentComponent *Equipment = Character->EquipmentComponent;
    DamageResistance = BonusDamageResistance + Equipment->EqipmentDamageResistance;
}

void UAttributeComponent::UpdateDamageBonus()
{
    auto Character = Cast<ACharacterBase>(GetOwner());
    const UEquipmentComponent *Equipment = Character->EquipmentComponent;
    DamageBonus  = BonusDamageBonus + Equipment->EqipmentDamageBonus;
}

void UAttributeComponent::UpdateHealth()
{
    float hk = Health->CurrentValue / Health->GetMaxValue();
    float LevelHealthBonus = 0;
    const auto LevelNum = LevelList.Num();
    for (auto &levelInfo : LevelList)
        LevelHealthBonus += levelInfo.HealthBonus;
    auto ConMod = UAttributeSetHelper::GetModifier(Attributes, EAttributes::Con);
    if (ConMod < 0)
        ConMod = 0;    // todo нужно ли?
    Health->BaseValue = LevelHealthBonus + (ConMod * LevelNum);
    Health->CurrentValue = Health->GetMaxValue() * hk;
}

void UAttributeComponent::UpdateHealthRegen()
{
    auto ConMod = UAttributeSetHelper::GetModifier(Attributes, EAttributes::Con);
    if (ConMod < 0)
        ConMod = 0;    // todo нужно ли?
    Health->RegenValue = BonusHealthRegen + ConMod;
}

void UAttributeComponent::UpdateStamina()
{
    const auto LevelNum = LevelList.Num();
    float sk = Stamina->CurrentValue / Stamina->GetMaxValue();
    auto ConMod = UAttributeSetHelper::GetModifier(Attributes, EAttributes::Con);
    if (ConMod < 0)
        ConMod = 0;    // todo нужно ли?
    auto StrMod = UAttributeSetHelper::GetModifier(Attributes, EAttributes::Str);
    if (StrMod < 0)
        StrMod = 0;
    Stamina->BaseValue = (LevelNum * ConMod) + (LevelNum * StrMod);
    Stamina->CurrentValue = Stamina->GetMaxValue() * sk;
}

void UAttributeComponent::UpdateStaminaRegen()
{
    auto ConMod = UAttributeSetHelper::GetModifier(Attributes, EAttributes::Con);
    if (ConMod < 0)
        ConMod = 0;    // todo нужно ли?
    auto StrMod = UAttributeSetHelper::GetModifier(Attributes, EAttributes::Str);
    if (StrMod < 0)
        StrMod = 0;
    Stamina->RegenValue = BonusStaminaRegen + StrMod + ConMod;
}

float UAttributeComponent::TakeDamage(float DamageAmount, UDamageType *DamageType, AActor *DamageCauser)
{
    const auto PhysD  = Cast<UPhysicDamage>(DamageType);
    const auto MagicD = Cast<UMagicDamage>(DamageType);
    const auto AbsoluteD = Cast<UAbsoluteDamage>(DamageType);

    if(AbsoluteD)
    {
        Health->Consume(DamageAmount);
        return Health->CurrentValue;
    }

    for(auto Barrier : BarrierList)
    {
        check(Barrier != nullptr && "Some Barrier is Invalid");
        if(DamageType->IsA(Barrier->DamageClass))
        {
            DamageAmount = Barrier->TakeDamage(DamageAmount,DamageType,DamageCauser);
            if(DamageAmount <= 0)
                return Health->CurrentValue;
        }
    }

    if(PhysD)
    {
        auto Type  = PhysD->Type;
        auto Force = PhysD->Force;
        //TODO: Use Force, Luke
        DamageAmount -= DamageAbsorb.DamageMap[(int32)Type];
        DamageAmount = DamageAmount * (DamageResistance.DamageMap[(int32)Type] - 100) / 100;
        Health->Consume(DamageAmount);
    }
    else if(MagicD)
    {
        auto Type        = MagicD->Type;
        DamageAmount -= DamageAbsorb.DamageMap[(int32)Type];
        DamageAmount = DamageAmount * (DamageResistance.DamageMap[(int32)Type] - 100) / 100;
        Health->Consume(DamageAmount);
    }
    return Health->CurrentValue;
}

void UAttributeComponent::Update()
{
    auto Character = Cast<ACharacterBase>(GetOwner());
    const UEquipmentComponent *Equipment = Character->EquipmentComponent;

    float hk = Health->CurrentValue / Health->GetMaxValue();
    float sk = Stamina->CurrentValue / Stamina->GetMaxValue();

    check(CharacterRace != nullptr && "CharacterRace is Invalid");

    BaseAttribute = CharacterRace->Attributes;
    auto LevelSkills = FSkillList();
    float LevelHealthBonus = 0;
    const auto LevelNum = LevelList.Num();
    for (auto &levelInfo : LevelList)
    {
        LevelHealthBonus += levelInfo.HealthBonus;
        BaseAttribute += levelInfo.Attributes;
        LevelSkills += levelInfo.SkillList;
    }

    Attributes = BaseAttribute + BonusAttribute + Equipment->EquipmentAttributes;
    Skills = BonusSkills + LevelSkills + Equipment->EquipmentSkills;
    DamageResistance = BonusDamageResistance + Equipment->EqipmentDamageResistance;
    DamageAbsorb = BonusDamageAbsorb + Equipment->EqipmentDamageAbsorb;
    DamageBonus  = BonusDamageBonus + Equipment->EqipmentDamageBonus;

    auto ConMod = UAttributeSetHelper::GetModifier(Attributes, EAttributes::Con);
    if (ConMod < 0)
        ConMod = 0;
    auto StrMod = UAttributeSetHelper::GetModifier(Attributes, EAttributes::Str);
    if (StrMod < 0)
        StrMod = 0;

    Health->BaseValue = 1.f + LevelHealthBonus + (ConMod * LevelNum);
    Health->CurrentValue = Health->GetMaxValue() * hk;

    Stamina->BaseValue = 1.f + (LevelNum * ConMod) + (LevelNum * StrMod);
    Stamina->CurrentValue = Stamina->GetMaxValue() * sk;

    Health->RegenValue = BonusHealthRegen + ConMod;
    Stamina->RegenValue = BonusStaminaRegen + StrMod;

    bIsDirty = false;
}

bool UAttributeComponent::SkillCheck(ESkill Skill, int32 value)
{
    return (Skills.SkillMap[(int32)Skill] +
            UAttributeSetHelper::GetModifier(Attributes, USkillHelper::GetModifier(Skill))) >= value;
}

void UAttributeComponent::ModifyAttribute(FAttributeSet Attribute)
{
    BonusAttribute += Attribute;
    Update();
}

void UAttributeComponent::ModifySkills(FSkillList InSkills)
{
    BonusSkills += InSkills;
    UpdateSkills();
}

void UAttributeComponent::ModifyDamageAbsorb(FDamageAbsorb DAbsorb)
{
    BonusDamageAbsorb += DAbsorb;
    UpdateDamageAbsorb();
}

void UAttributeComponent::ModifyDamageResist(FDamageResistance DamageResist)
{
    DamageResist += DamageResist;
    UpdateDamageResist();
}

void UAttributeComponent::ModifyDamageBonus(FDamageAbsorb DBonus)
{
    BonusDamageBonus += DBonus;
    UpdateDamageBonus();
}

void UAttributeComponent::ModifyHealthBonus(float HPBonus)
{
    BonusHealth += HPBonus;
    UpdateHealth();
}

void UAttributeComponent::ModifyHealthRegen(float HPRegen)
{
    BonusHealthRegen += HPRegen;
    UpdateHealthRegen();
}

void UAttributeComponent::ModifyStaminaBonus(float STBonus)
{
    BonusStamina += STBonus;
    UpdateStamina();
}

void UAttributeComponent::ModifyStaminaRegen(float STRegen)
{
    BonusStaminaRegen += STRegen;
    UpdateStaminaRegen();
}

const ACharacterBase *UAttributeComponent::GetCharacterOwner() const
{
    return Cast<ACharacterBase>(GetOwner());
}

#if WITH_EDITOR
void UAttributeComponent::PostEditChangeProperty(FPropertyChangedEvent &e)
{
    Super::PostEditChangeProperty(e);

    FName PropertyName = (e.Property != nullptr) ? e.Property->GetFName() : NAME_None;
    if(PropertyName == GET_MEMBER_NAME_CHECKED(UAttributeComponent, LevelList))
    {
        Update();
    }
    else if(PropertyName == GET_MEMBER_NAME_CHECKED(UAttributeComponent, CharacterRace))
    {
        Update();
    }
}
#endif

void UAttributeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    Health->ClearRegenTimer();
    Stamina->ClearRegenTimer();
}

// Called when the game starts
void UAttributeComponent::BeginPlay()
{
    Super::BeginPlay();
    auto Character = Cast<ACharacterBase>(GetOwner());
    //Health = NewObject<UDynamicAttribute>();
    //Stamina = NewObject<UDynamicAttribute>();

    check(Character != nullptr && "UAttributeComponent error: No pawn");
    check(Health != nullptr && "UAttributeComponent error: Health is Invalid");
    check(Stamina != nullptr && "UAttributeComponent error: Stamina is Invalid");
    check(Character->GetWorld() != nullptr && "UAttributeComponent error: World is Invalid")
    //Это жёсткий костыль, потому что при CreateDefaultSubobject в конструкторе он неправильно
    //ставит оутера или ворлд, я хз прочему, поэтому в начале игры я вручную ставлю
    Health->SetWorld(Character->GetWorld());
    Stamina->SetWorld(Character->GetWorld());

    Health->StartRegenTimer();
    Stamina->StartRegenTimer();
    Stamina->CurrentValue = Stamina->GetMaxValue();
}

// Called every frame
void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // ...
}



// Sets default values for this component's properties
UAttributesComponent::UAttributesComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    /* Some shitty ways to init uObjects
	//Health = CreateDefaultSubobject<UHealthContainer>("Health");
	//Stamina = CreateDefaultSubobject<UStaminaContainer>("Stamina");
	//MagicShield = CreateDefaultSubobject<UMagicShieldContainer>("MShield");
		HealthClass = UHealthContainer::StaticClass();
	StaminaClass = UStaminaContainer::StaticClass();
	MagicShieldClass = UMagicShieldContainer::StaticClass();
	check(HealthClass->IsValidLowLevelFast());
	check(StaminaClass->IsValidLowLevelFast());
	check(MagicShieldClass->IsValidLowLevelFast());
	Health = NewObject<UHealthContainer>(this, HealthClass->GetFName(), RF_NoFlags, HealthClass->GetDefaultObject());
	Stamina = NewObject<UStaminaContainer>(this, StaminaClass->GetFName(), RF_NoFlags, StaminaClass->GetDefaultObject());
	MagicShield = NewObject<UMagicShieldContainer>(this, MagicShieldClass->GetFName(), RF_NoFlags, MagicShieldClass->GetDefaultObject());
	*/

    // ...
}

void UAttributesComponent::Update()
{
    float hk = Health->CurrentVal / Health->CurrentMaxVal;
    float sk = Stamina->CurrentVal / Stamina->CurrentMaxVal;

    //CurrentAttributes = BaseAttributes + Character->ExperienceComponent->LevelAttributes;    // + equipment
    ///CurrentSkills = BaseSkills + Character->ExperienceComponent->LevelSkills;    // + equipment
    CurrentDamageResistance = BaseDamageResistance;    // + Other Damage Resistance(equipment)
    CurrentDamageAbsorb = BaseDamageAbsorb;    // + Other Damage Absorb(equipment)
    CurrentDamageBonus  = BaseDamageBonus;    // + Other Damage Bonus(equipment)

    auto ConMod = UAttributeSetHelper::GetModifier(CurrentAttributes, EAttributes::Con);
    if (ConMod < 0)
        ConMod = 0;    // todo нужно ли?

    //Health->CurrentMaxVal = Health->BaseMaxVal + Character->ExperienceComponent->LevelHealthBonus
    //                        + (ConMod * Character->ExperienceComponent->GetLevelNum());
    ///Health->CurrentVal = Health->CurrentMaxVal * hk;

    auto StrMod = UAttributeSetHelper::GetModifier(CurrentAttributes, EAttributes::Str);
    if (StrMod < 0)
        StrMod = 0;    // todo нужно ли?
    //Stamina->CurrentMaxVal = Stamina->BaseMaxVal
     //                         + (Character->ExperienceComponent->GetLevelNum() * ConMod)
     //                         + (Character->ExperienceComponent->GetLevelNum() * StrMod);
    Stamina->CurrentVal = Stamina->CurrentMaxVal * sk;

    Health->SetCurrentRegenRate(Health->GetBaseRegenRate() + ConMod);
    Stamina->SetCurrentRegenRate(Stamina->GetBaseRegenRate() + ConMod);

    UnSetUpdate();
}

// Called when the game starts
void UAttributesComponent::BeginPlay()
{
    Super::BeginPlay();
    Character = Cast<ACharacterBase>(GetOwner());
    check(Character != nullptr && "UBaseMovementComponent error: No pawn");

    Health->OnRegenTick();
    Stamina->OnRegenTick();
    // ...
}

// Called every frame
void UAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

