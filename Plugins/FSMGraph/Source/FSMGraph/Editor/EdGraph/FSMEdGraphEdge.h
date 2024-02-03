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
	TSharedPtr<SGraphNode> CreateVisualWidget() override;
	void AllocateDefaultPins() override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	void PinConnectionListChanged(UEdGraphPin *Pin) override;
	void PrepareForCopying() override;

	UEdGraphPin *GetInputPin() const {return Pins[0];}
	UEdGraphPin *GetOutputPin() const {return Pins[1];}

	UFSMEdGraphNode *GetPreviousState() const;
	UFSMEdGraphNode *GetNextState() const;
	void CreateConnections(UFSMEdGraphNode *PrevState, UFSMEdGraphNode *NextState);

	FName startName;
	FName endName;

private:
	TSharedPtr<SGraphNode> SlateNode;
};