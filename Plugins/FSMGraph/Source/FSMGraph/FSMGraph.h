#pragma once

#include "CoreMinimal.h"
#include "FSMGraph.generated.h"


DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FSMCondition, class ACharacter*, Character, class UFSMGraph *, BaseParams);


USTRUCT(BlueprintType)
struct FSMLink
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 LinkedStateID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FSMCondition ConditionDelegate;
};

USTRUCT(BlueprintType)
struct FSMState
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<FSMLink> Links;
    
    

    friend class UFSMGraph;
};

UCLASS(BlueprintType)
class UFSMGraph : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY()
    UEdGraph *EdGraph;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<FSMState> TestData;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TMap<FName, FSMState> StatesMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    FName InitStateName;

    void AddState();



    UFUNCTION()
    void Modify() {}
};


