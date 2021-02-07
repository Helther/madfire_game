// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Rules/GameRules.h"
#include "Engine/UserDefinedStruct.h"

#include "Attributes.generated.h"

/**
 * 
 */


USTRUCT(BlueprintType)
struct FAttributes
{
    GENERATED_BODY()

    FAttributes();

    FORCEINLINE FAttributes &operator+=(const FAttributes &attributes)
    {
        for (auto i = 0; i < (int32)EAttributes::Max; ++i)
            AttributeMap[i] += attributes.AttributeMap[i];
        return *this;
    }

    friend FORCEINLINE FAttributes operator+(const FAttributes &lhs,
                                             const FAttributes &rhs)
    {
        FAttributes result = lhs;
        result += rhs;
        return result;
    }


    UPROPERTY(EditAnywhere, Category = Impact)
    uint8 AttributeMap[(int32)EAttributes::Max];
};

UCLASS()
class UAttributesHelper : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Attributes")
    static FORCEINLINE int32 GetAttribute(const FAttributes &attr,
                                          EAttributes AttributeName)
    {
        return attr.AttributeMap[(int32)AttributeName];
    }

    UFUNCTION(BlueprintPure, Category = "Attributes")
    static FORCEINLINE int32 GetModifier(const FAttributes &attr,
                                         EAttributes AttributeName)
    {
        return (attr.AttributeMap[(int32)AttributeName] - 10) / 2;
    }

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    static void SetAttribute(FAttributes &attr, EAttributes AttributeName, int32 Value)
    {
        attr.AttributeMap[(int32)AttributeName] = (uint8)Value;
    }
};
