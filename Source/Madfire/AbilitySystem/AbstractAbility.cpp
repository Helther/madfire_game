#include "AbstractAbility.h"

bool UAbstractAbility::CheckRequirements_Implementation(void)
{
	if (Requirements.Num() == 0)
	{
		UE_LOG(AbilitySystemLog, Warning, TEXT("Requirements: is not set up"));
		return true;
	}

	for (auto req : Requirements)
	{
		if (!req->CheckRequirement(Caster))
		{
			return false;
		}
	}

	return true;
}

UAbstractAbility *UAbilityManager::CreateSkill(ACharacter *caster, TSubclassOf<UAbstractAbility> SkillClass)
{
	UAbstractAbility *skill = SkillClass.GetDefaultObject();
	check(skill != nullptr && "Ability Manager: skill is invalid")
	skill->Caster = caster;
	//for debug only
	skill->CurrentCharges = skill->MaxCharges;
	return skill;
}

AAbilityComponent *UAbilityManager::SpawnComponent(AActor *target, FAbilityComponentSpawnParameters SpawnParameters, int impower)
{
	return SpawnParameters.Spawn(target);
}




