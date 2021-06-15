// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Rules/GameRules.h"
#include "Engine/UserDefinedStruct.h"

#include "Attributes.generated.h"

/**
 * 
 */


USTRUCT(BlueprintType)
struct FAttributeSet
{
    GENERATED_BODY()

    FAttributeSet();

    FORCEINLINE FAttributeSet &operator+=(const FAttributeSet &attributes)
    {
        for (auto i = 0; i < (int32)EAttributes::Max; ++i)
            AttributeMap[i] += attributes.AttributeMap[i];
        return *this;
    }

    friend FORCEINLINE FAttributeSet operator+(const FAttributeSet &lhs, const FAttributeSet &rhs)
    {
        FAttributeSet result = lhs;
        result += rhs;
        return result;
    }


    UPROPERTY(SaveGame, EditAnywhere, Category = Impact)
    uint8 AttributeMap[(int32)EAttributes::Max];
};

UCLASS(MinimalAPI, Abstract)
class UAttributeSetHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Attributes")
    static FORCEINLINE int32 GetAttribute(const FAttributeSet &attr, EAttributes AttributeName)
    {
        return attr.AttributeMap[(int32)AttributeName];
    }

    UFUNCTION(BlueprintPure, Category = "Attributes")
    static FORCEINLINE int32 GetModifier(const FAttributeSet &attr, EAttributes AttributeName)
    {
        return (attr.AttributeMap[(int32)AttributeName] - 10) / 2;
    }

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    static void SetAttribute(FAttributeSet &attr, EAttributes AttributeName, int32 Value)
    {
        attr.AttributeMap[(int32)AttributeName] = (uint8)Value;
    }
};
