// Fill out your copyright notice in the Description page of Project Settings.

#include "Skills.h"

const EAttributes FSkillList::ModifierMap[(int32)ESkill::Max] = {
    EAttributes::Int,    //Appraise
    EAttributes::Cha,    //Bluff
    EAttributes::Cha,    //Intimidate
    EAttributes::Cha,    //Diplomacy
    EAttributes::Cha,    //Perform
    EAttributes::Wis,    //Perception
    EAttributes::Dex,    //SleightOfHand
    EAttributes::Dex,    //OpenLock
    EAttributes::Int,    //Knowledge
    EAttributes::Dex,    //Sneakiness
    EAttributes::Dex,    //MoveSilently
    EAttributes::Wis,    //Listen
    EAttributes::Dex,    //Balance
    EAttributes::Con,    //Concetration
    EAttributes::Dex,    //Tumble
    EAttributes::Wis,    //Survival
    EAttributes::Str,    //Blacksmithing
    EAttributes::Int,    //Alchemy
    EAttributes::Int,    //Enchantment
    EAttributes::Cha    //HandleAnimal
};

FSkillList::FSkillList()
  : SkillMap{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
{}

FSkillList &FSkillList::operator+=(const FSkillList &rhs)
{
    for (auto i = 0; i < (int32)ESkill::Max; ++i)
        SkillMap[i] += rhs.SkillMap[i];
    return *this;
}

FSkillList FSkillList::operator+(const FSkillList &rhs)
{
    FSkillList result(rhs);
    result += *this;
    return result;
}

void USkillHelper::SetValue(FSkillList &skill, ESkill SkillName, int32 value)
{
    skill.SkillMap[(int32)SkillName] = (int8)value;
}
