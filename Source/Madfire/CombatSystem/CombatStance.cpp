// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatStance.h"

FAttackInfo* UCombatStance::GetAttackInfoForSpecial(ESpecialType type)
{
	return SpecialAttacks.FindByPredicate([type](const FAttackInfo& info) { return info.Special == type; });
}
