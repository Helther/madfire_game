// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageMap.h"

/*
FDamageMap::FDamageMap(const FDamageMap &resistance)
{
	for (int i = 0; i < (int32)EDamageType::Max; ++i)
		DamageMap[i] = resistance.DamageMap[i];
}
*/
FDamageResistance &FDamageResistance::operator+=(const FDamageResistance &resistance)
{
	for (int i = 0; i < (int32)EDamageType::Max; ++i)
		DamageMap[i] += resistance.DamageMap[i];
	return *this;
}

FDamageResistance FDamageResistance::operator+(const FDamageResistance &resistance)
{
	FDamageResistance temp(*this);
	temp += resistance;
	return temp;
}

FDamageAbsorb &FDamageAbsorb::operator+=(const FDamageAbsorb &resistance)
{
	for (int i = 0; i < (int32)EDamageType::Max; ++i)
		DamageMap[i] += resistance.DamageMap[i];
	return *this;
}

FDamageAbsorb FDamageAbsorb::operator+(const FDamageAbsorb &resistance)
{
	FDamageAbsorb temp(*this);
	temp += resistance;
	return temp;
}

