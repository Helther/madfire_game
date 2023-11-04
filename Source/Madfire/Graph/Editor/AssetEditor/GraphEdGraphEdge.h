#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "GraphEdGraphEdge.generated.h"

class UGraphNode;
class UGraphEdge;
class UGraphEdGraphNode;

UCLASS(MinimalAPI)
class UGraphEdGraphEdge : public UEdGraphNode
{
    GENERATED_BODY()

public:
    UPROPERTY()
    class UEdGraph *Graph;

    UPROPERTY(VisibleAnywhere, Instanced, Category = "Graph")
    UGraphEdge *GraphEdge;

    void SetEdge(UGraphEdge *Edge);

    void AllocateDefaultPins() override;
    FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    void PinConnectionListChanged(UEdGraphPin *Pin) override;
    void PrepareForCopying() override;

    virtual UEdGraphPin *GetInputPin() const { return Pins[0]; }
    virtual UEdGraphPin *GetOutputPin() const { return Pins[1]; }

    void CreateConnections(UGraphEdGraphNode *Start, UGraphEdGraphNode *End);

    UGraphEdGraphNode *GetStartNode();
    UGraphEdGraphNode *GetEndNode();
};