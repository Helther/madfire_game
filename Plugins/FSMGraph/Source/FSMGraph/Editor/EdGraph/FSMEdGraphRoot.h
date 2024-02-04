#pragma once

#include "FSMEdGraphNode.h"
#include "SubclassOf.h"
#include "FSMEdGraphRoot.generated.h"


class UFSMGraph;
class UFSMEdGraphEdge;


UCLASS(MinimalAPI)
class UFSMEdGraphRoot : public UFSMEdGraphNode
{
	GENERATED_BODY()

public:
	UFSMEdGraphRoot();
	~UFSMEdGraphRoot() override;

	TSharedPtr<SGraphNode> CreateVisualWidget() override;
	void AllocateDefaultPins() override;

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	void PrepareForCopying() override;
	void DestroyNode() override;
	UEdGraphNode *GetOutputNode() const;

	UEdGraphPin *GetOutputPin() const override {return Pins[0]; }
	UEdGraphPin *GetInputPin() const override { return Pins[0]; }
};