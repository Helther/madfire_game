#include "AbilityAction.h"
#include "AbilitySystemLog.h"

bool UAbilityActionCollection::Execute_Implementation(AActor *Target, int32 Empowerment)
{
	check(Target != nullptr && "Action collection: target not found");

	for (auto action : ActionList)
	{
		auto obj = action.GetDefaultObject();
		check(obj != nullptr && "Action collection: action is not valid");
		obj->Caster = Caster;
		obj->Causer = Causer;
		obj->Execute(Target);
	}

	UE_LOG(AbilitySystemLog, Warning, TEXT("Action collection: all actions is done"));
	return true;
}
