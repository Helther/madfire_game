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

    UPROPERTY(EditAnywhere,Category = Functions, meta=(FunctionReference, AllowFunctionLibraries, PrototypeFunction="/Script/Madfire.CharacterBase.SetupItemActorCollision", DefaultBindingName="OnUpdate"), DisplayName="On Update")
    FMemberReference OnUpdate;

    UPROPERTY(EditAnywhere,Category = Functions, meta=(FunctionReference, AllowFunctionLibraries, PrototypeFunction="/Script/Madfire.CharacterBase.SetupItemActorCollision", DefaultBindingName="OnUpdate"), DisplayName="On Update")
    FMemberReference OnEnter;

    UPROPERTY(EditAnywhere,Category = Functions, meta=(FunctionReference, AllowFunctionLibraries, PrototypeFunction="/Script/Madfire.CharacterBase.SetupItemActorCollision", DefaultBindingName="OnUpdate"), DisplayName="On Update")
    FMemberReference OnLeave;
    
    friend class UFSMGraph;
};

UCLASS(BlueprintType)
class UFSMGraph : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY()
    UEdGraph *EdGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FSMState> StatesMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InitStateName;

    FName SpawnNode();

    FSMState &GetState(const FName &);
    void AddState(const FName &, FSMState &);
    void RemoveState(const FName &);

    bool Validate(const FName &);

    UFUNCTION()
    void Modify() {}


};


