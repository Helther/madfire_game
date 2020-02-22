
#include "LevelList.h"


FLevelInfo::FLevelInfo() :
	hpBonus(DefaultHPBonus)
{}



ULevelComponent::ULevelComponent() 
{
	LevelList.Reserve(MaxLevel);
}

void ULevelComponent::ResetLevel()
{
	LevelList.Empty(MaxLevel);
}

void ULevelComponent::LevelUp(const FLevelInfo &levelInfo)
{
	check((LevelList.Num() <= MaxLevel) && "Level limit is 30");

	LevelList.Add(levelInfo);

	EXPERIENCE_FUCKING_EQUATION(RequiredExpirience); //TODO: Exp Equation
}

void ULevelComponent::update()
{
	LevelAttributes = FAttributes();
	LevelSkills = FSkillList();
	
	for (auto &levelInfo : LevelList)
	{
		LevelHealthBonus += levelInfo.hpBonus;
		LevelAttributes += levelInfo.Attributes;
		LevelSkills += levelInfo.SkillList;
		//Talents
	}
}
