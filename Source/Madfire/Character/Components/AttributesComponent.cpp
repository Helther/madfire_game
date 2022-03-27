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
    SetDirty();
}

float UAttributeComponent::TakeDamage(float DamageAmount, UDamageType *DamageType, AActor *DamageCauser)
{
    const auto PhysD  = Cast<UPhysicDamage>(DamageType);
    const auto MagicD = Cast<UMagicDamage>(DamageType);
    const auto AbsoluteD = Cast<UAbsoluteDamage>(DamageType);

    if(AbsoluteD)
    {
        Health->Consume(DamageAmount);
        GetCharacterOwner()->Logger.Log(this,*FString::Printf(TEXT("take damage after attributes %f"), DamageAmount));
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
        auto Type = PhysD->Type;
        DamageAmount -= DamageAbsorb.DamageMap[(int32)Type];
        DamageAmount = DamageAmount * (100 - DamageResistance.DamageMap[(int32)Type]) / 100;
        Health->Consume(DamageAmount);
    }
    else if(MagicD)
    {
        auto Type = MagicD->Type;
        DamageAmount -= DamageAbsorb.DamageMap[(int32)Type];
        DamageAmount = DamageAmount * (100 - DamageResistance.DamageMap[(int32)Type]) / 100;
        Health->Consume(DamageAmount);
    }
    GetCharacterOwner()->Logger.Log(this,*FString::Printf(TEXT("take damage after attributes %f"), DamageAmount));
    return Health->CurrentValue;
}

void UAttributeComponent::Update()
{
    auto Character = GetCharacterOwner();

    float hk = Health->CurrentValue / Health->GetMaxValue();
    float sk = Stamina->CurrentValue / Stamina->GetMaxValue();
    
    BaseAttribute = FAttributeSet();
    auto LevelSkills = FSkillList();
    float LevelHealthBonus = 0;
    float LevelStaminaBonus = 0;
    float LevelHealthRegenBonus = 0;
    float LevelStaminaRegenBonus = 0;
    FDamageAbsorb LevelDamageAbsorbBonus;
    FDamageAbsorb LevelDamageBonus;
    FDamageResistance LevelDamageResistanceBonus;
    const auto LevelNum = LevelList.Num();
    for (auto &levelInfo : LevelList)
    {
        BaseAttribute += levelInfo.Attributes;
        LevelSkills += levelInfo.SkillList;
        LevelHealthBonus += levelInfo.HealthBonus;
        LevelStaminaBonus += levelInfo.StaminaBonus;
        LevelHealthRegenBonus += levelInfo.HealthRegenBonus;
        LevelStaminaRegenBonus += levelInfo.StaminaRegenBonus;
        LevelDamageAbsorbBonus += levelInfo.DamageAbsorbBonus;
        LevelDamageBonus += levelInfo.DamageBonus;
        LevelDamageResistanceBonus += levelInfo.DamageResistanceBonus;
    }

    Attributes = BaseAttribute + BonusAttribute;
    Skills = BonusSkills + LevelSkills;

    auto ConMod = UAttributeSetHelper::GetModifier(Attributes, EAttributes::Con);
    if (ConMod < 0)
        ConMod = 0;
    auto StrMod = UAttributeSetHelper::GetModifier(Attributes, EAttributes::Str);
    if (StrMod < 0)
        StrMod = 0;

    Health->BaseValue = 1.f + LevelHealthBonus + (ConMod * LevelNum) + BonusHealth;
    Health->CurrentValue = Health->GetMaxValue() * hk;

    Stamina->BaseValue = 1.f + (LevelNum * ConMod) + (LevelNum * StrMod) + LevelStaminaBonus + BonusStamina;
    Stamina->CurrentValue = Stamina->GetMaxValue() * sk;

    Health->RegenValue = LevelHealthRegenBonus + BonusHealthRegen + ConMod;
    Stamina->RegenValue = LevelStaminaRegenBonus + BonusStaminaRegen + StrMod;

    DamageAbsorb = LevelDamageAbsorbBonus + BonusDamageAbsorb;
    DamageBonus = LevelDamageBonus + BonusDamageBonus;
    DamageResistance = LevelDamageResistanceBonus + BonusDamageResistance;

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
    CallUpdate();
}

void UAttributeComponent::ModifySkills(FSkillList SkillsBonus)
{
    BonusSkills += SkillsBonus;
    CallUpdate();
}

void UAttributeComponent::ModifyDamageAbsorb(FDamageAbsorb DAbsorb)
{
    BonusDamageAbsorb += DAbsorb;
    CallUpdate();
}

void UAttributeComponent::ModifyDamageResist(FDamageResistance DResist)
{
    BonusDamageResistance += DResist;
    CallUpdate();
}

void UAttributeComponent::ModifyDamageBonus(FDamageAbsorb DBonus)
{
    BonusDamageBonus += DBonus;
    CallUpdate();
}

void UAttributeComponent::ModifyHealthBonus(float HPBonus)
{
    BonusHealth += HPBonus;
    CallUpdate();
}

void UAttributeComponent::ModifyHealthRegen(float HPRegen)
{
    BonusHealthRegen += HPRegen;
    CallUpdate();
}

void UAttributeComponent::ModifyStaminaBonus(float STBonus)
{
    BonusStamina += STBonus;
    CallUpdate();
}

void UAttributeComponent::ModifyStaminaRegen(float STRegen)
{
    BonusStaminaRegen += STRegen;
    CallUpdate();
}

const ACharacterBase *UAttributeComponent::GetCharacterOwner() const
{
    return Cast<ACharacterBase>(GetOwner());
}

ACharacterBase *UAttributeComponent::GetCharacterOwner()
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

void UAttributeComponent::CallUpdate()
{
    auto Character = GetCharacterOwner();
    bIsDirty = true;
    Character->Update(FCharacterLevelInfo());
}

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
    auto Character = GetCharacterOwner();
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
