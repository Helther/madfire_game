#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "GraphEdGraph.generated.h"

class UGraph;
class UGraphNode;
class UGraphEdge;
class UGraphEdGraphNode;
class UGraphEdGraphEdge;

UCLASS()
class UGraphEdGraph : public UEdGraph
{
    GENERATED_BODY()
public:
    UGraphEdGraph();
    ~UGraphEdGraph();

    virtual void RebuildGraph();

    UGraph *GetGraph() const;

    bool Modify(bool bAlwaysMarkDirty = true) override;
    void PostEditUndo() override;

    UPROPERTY(Transient)
    TMap<UGraphNode*, UGraphEdGraphNode*> NodeMap;

    UPROPERTY(Transient)
    TMap<UGraphNode*, UGraphEdGraphEdge*> EdgeMap;

protected:
    void Clear();
    void SortNodes(UGraphNode *RootNode);


};