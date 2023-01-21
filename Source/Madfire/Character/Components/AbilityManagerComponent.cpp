#include "AbilityManagerComponent.h"
#include "Character/CharacterBase.h"



UAbilityManagerComponent::UAbilityManagerComponent(const FObjectInitializer &ObjectInitializer)
{

}

void UAbilityManagerComponent::Update()
{
    auto character = Cast<ACharacterBase>(GetOwner());
    auto attributes = character->AttributeComponent;
    auto intelect = attributes->Attributes.AttributeMap[(int32)EAttributes::Int];
    auto wisdom = attributes->Attributes.AttributeMap[(int32)EAttributes::Wis];

    LevelSpellList.SetNum(wisdom,true);
    for(auto &list : LevelSpellList)
        list.SpellList.SetNum(intelect,true);

    AbilityList.Empty();

    AbilityList.Append(ExtraAbilityList);
    AbilityList.Append(SpellAbilityList);
    AbilityList.Append(EquipAbilityList);
    AbilityList.Append(TalentAbilityList);
}

UAbstractAbility *UAbilityManagerComponent::GetAbility(FName AbilityName)
{
    if(!AbilityList.Contains(AbilityName))
        return nullptr;
    return AbilityList[AbilityName];
} 

void UAbilityManagerComponent::UpdateSpellBook()
{
    SpellAbilityList.Empty();

    auto owner = GetOwner();
 
    for(const auto &spellLevel : LevelSpellList)
    {
       
        for(const auto &spell : spellLevel.SpellList)
        {
            //if(!spell) continue;

            const auto &info = spell.GetAbilityInfo();

            if(!info.AbilityClass)
                continue;
            auto abl = UAbstractAbility::SpawnAbility(owner,info.AbilityClass);
            if(abl)
            {
                const auto name = abl->AbilityName;
                if (SpellAbilityList.Contains(name))
                {
                    SpellAbilityList[name]->MaxCharges += abl->GetInfo().MaxCharges;
                    SpellAbilityList[name]->CurrentCharges = SpellAbilityList[name]->MaxCharges;
                      
                }
                else
                {
                    SpellAbilityList.Add(abl->AbilityName, abl);
                }
            }
        }
    }

    Update();
}
