#include "ExperienceComponent.h"


FLevelInfo::FLevelInfo() : hpBonus(DefaultHPBonus) {}


UExperienceComponent::UExperienceComponent(const class FObjectInitializer &ObjectInitilizer)
  : Super(ObjectInitilizer)
{
    LevelList.Reserve(MaxLevel);
}

void UExperienceComponent::ResetLevel() { LevelList.Empty(MaxLevel); }

void UExperienceComponent::LevelUp(const FLevelInfo &levelInfo)
{
    check((LevelList.Num() <= MaxLevel) && "Level limit is 30");

    LevelList.Add(levelInfo);

    EXPERIENCE_FUCKING_EQUATION(RequiredExperience);    //TODO: Exp Equation
}

void UExperienceComponent::Update()
{
    LevelAttributes  = FAttributes();
    LevelSkills      = FSkillList();
    LevelHealthBonus = 0;

    for (auto &levelInfo : LevelList)
    {
        LevelHealthBonus += levelInfo.hpBonus;
        LevelAttributes += levelInfo.Attributes;
        LevelSkills += levelInfo.SkillList;
        //Talents
    }
    UnSetUpdate();
}
