#pragma once


#include "CoreMinimal.h"
#include "CondGraph.generated.h"


UCLASS(MinimalAPI)
class UFSMCondGraph: public UEdGraph
{
	GENERATED_BODY()

public:
	class UFSMConditionResultNode* GetResultNode();

	UPROPERTY()
	TObjectPtr<class UFSMConditionResultNode> MyResultNode;
};