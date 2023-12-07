#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "SubclassOf.h"
#include "FSMEdGraphEdge.generated.h"

class UFSMGraph;
class UFSMEdGraphNode;

UCLASS(MinimalAPI)
class UFSMEdGraphEdge : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UFSMEdGraphEdge();
	~UFSMEdGraphEdge() override;

	//Node interface
	void AllocateDefaultPins() override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	void PinConnectionListChanged(UEdGraphPin *Pin) override;
	void PrepareForCopying() override;

	UEdGraphPin *GetInputPin() const {return Pins[0];}
	UEdGraphPin *GetOutputPin() const {return Pins[1];}

	void CreateConnections(UFSMEdGraphNode *Start, UFSMEdGraphNode *End);

	UFSMEdGraphNode *GetStartNode();
	UFSMEdGraphNode *GetEndNode();

	FName startName;
	FName endName;
};