#include "AbstractSpell.h"


FSpell::FSpell()
{

}

const FAbilityInfo &FSpell::GetAbilityInfo() const
{
    return UAbstractAbility::GetAbilityInfo(InfoDataTable, AbilityName);
}
