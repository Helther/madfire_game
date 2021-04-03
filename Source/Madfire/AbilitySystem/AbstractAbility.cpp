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

