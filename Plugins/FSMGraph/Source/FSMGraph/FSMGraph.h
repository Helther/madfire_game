#pragma once

#include "CoreMinimal.h"
#include "FSMGraph.generated.h"


USTRUCT(BlueprintType)
struct FSMLink
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 LinkedStateID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 ConditionDelegate;
};

USTRUCT(BlueprintType)
struct FSMState
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TMap<int32, int32> Links;
    
    friend class UFSMGraph;
};

UCLASS(BlueprintType)
class UFSMGraph : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY()
    UEdGraph *EdGraph;

    UPROPERTY()
    TArray<FSMState> TestData;

    UFUNCTION()
    void Modify() {}
};
