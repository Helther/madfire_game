#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "../../Graph.h"
#include "../AssetEditor/GraphEdGraph.h"
#include "../AssetEditor/GraphEdGraphEdge.h"
#include "../AssetEditor/GraphEdGraphNode.h"
#include "../AssetEditor/GraphAssetEditorSettings.h"
#include "AutoLayoutStrategy.generated.h"

UCLASS(abstract)
class UAutoLayoutStrategy : public UObject
{
    GENERATED_BODY()
public:
    UAutoLayoutStrategy();
    virtual ~UAutoLayoutStrategy();

    virtual void Layout(UEdGraph *G){};

    class UGraphAssetEditorSettings *Settings;

protected:
    int32 GetNodeWidth(UGraphEdGraphNode *EdNode);

    int32 GetNodeHeight(UGraphEdGraphNode *EdNode);

    FBox2D GetNodeBound(UEdGraphNode *EdNode);

    FBox2D GetActualBounds(UGraphNode *RootNode);

    virtual void RandomLayoutOneTree(UGraphNode *RootNode, const FBox2D &Bound);

protected:
    UGraph *Graph;
    UGraphEdGraph *EdGraph;
    int32 MaxIteration;
    int32 OptimalDistance;
};
