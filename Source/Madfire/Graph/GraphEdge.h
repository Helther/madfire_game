#pragma once

#include "CoreMinimal.h"
#include "GraphNode.h"
#include "GraphEdge.generated.h"


class UGraph;

UCLASS(Blueprintable)
class UGraphEdge : public UObject
{
    GENERATED_BODY()

public:
    UGraphEdge();
    ~UGraphEdge();

    UPROPERTY(VisibleAnywhere, Category = "Graph")
    UGraph *Graph;

    UPROPERTY(BlueprintReadOnly, Category = "Graph")
    UGraphNode *StartNode;

    UPROPERTY(BlueprintReadOnly, Category = "Graph")
    UGraphNode *EndNode;

    UFUNCTION(BlueprintPure, Category = "GenericGraphEdge")
    UGraph *GetGraph() const;

};
