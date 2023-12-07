#pragma once

#include "CoreMinimal.h"
#include "FSMGraph.generated.h"


DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FSMCondition, class ACharacter*, Character, class UFSMGraph *, BaseParams);


USTRUCT(BlueprintType)
struct FSMLink
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName LinkedStateID;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
    FSMCondition ConditionDelegate;
};

USTRUCT(BlueprintType)
struct FSMState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FSMState> TestData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FSMState> StatesMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InitStateName;

    FName SpawnNode();

    FSMState &GetState(FName &);
    void AddState(FName &, FSMState &);
    void RemoveState(FName &);

    bool Validate(FName &);

    UFUNCTION()
    void Modify() {}


};


