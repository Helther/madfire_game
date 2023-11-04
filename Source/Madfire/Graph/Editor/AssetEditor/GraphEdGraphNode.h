#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "../../GraphNode.h"
#include "GraphEdGraphNode.generated.h"

class UGraphEdGraphEdge;
class UGraphEdGraph;
class SGraphEdGraphNode;


UCLASS(MinimalAPI)
class UGraphEdGraphNode : public UEdGraphNode
{
    GENERATED_BODY()

public:
    UGraphEdGraphNode();
    ~UGraphEdGraphNode() override;

    UPROPERTY(VisibleAnywhere, Instanced, Category = "Graph")
    UGraphNode *GraphNode;

    void SetGraphNode(UGraphNode *Node);
    UGraphEdGraphNode *GetEdGraphNode();

    SGraphEdGraphNode *SEdNode;

    void AllocateDefaultPins() override;
    FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    void PrepareForCopying() override;
    void AutowireNewNode(UEdGraphPin *FromPin) override;

    FLinearColor GetBackgroundColor() const;
    UEdGraphPin *GetInputPin() const;
    UEdGraphPin *GetOutputPin() const;

#if WITH_EDITOR
    void PostEditUndo() override;
#endif 

};