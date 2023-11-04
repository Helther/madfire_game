#include "AutoLayoutStrategy.h"
#include "../AssetEditor/GraphEdGraphNode.h"
#include "../AssetEditor/GraphEdNodeSlateNode.h"
#include "Kismet/KismetMathLibrary.h"

UAutoLayoutStrategy::UAutoLayoutStrategy()
{
    Settings        = nullptr;
    MaxIteration    = 50;
    OptimalDistance = 150;
}

UAutoLayoutStrategy::~UAutoLayoutStrategy() {}

FBox2D UAutoLayoutStrategy::GetNodeBound(UEdGraphNode *EdNode)
{
    int32 NodeWidth  = GetNodeWidth(Cast<UGraphEdGraphNode>(EdNode));
    int32 NodeHeight = GetNodeHeight(Cast<UGraphEdGraphNode>(EdNode));
    FVector2D Min(EdNode->NodePosX, EdNode->NodePosY);
    FVector2D Max(EdNode->NodePosX + NodeWidth, EdNode->NodePosY + NodeHeight);
    return FBox2D(Min, Max);
}

FBox2D UAutoLayoutStrategy::GetActualBounds(UGraphNode *RootNode)
{
    int Level                                  = 0;
    TArray<UGraphNode *> CurrLevelNodes = { RootNode };
    TArray<UGraphNode *> NextLevelNodes;

    FBox2D Rtn = GetNodeBound(EdGraph->NodeMap[RootNode]);

    while (CurrLevelNodes.Num() != 0)
    {
        for (int i = 0; i < CurrLevelNodes.Num(); ++i)
        {
            UGraphNode *Node = CurrLevelNodes[i];
            check(Node != nullptr);

            Rtn += GetNodeBound(EdGraph->NodeMap[Node]);

            for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
            {
                NextLevelNodes.Add(Node->ChildrenNodes[j]);
            }
        }

        CurrLevelNodes = NextLevelNodes;
        NextLevelNodes.Reset();
        ++Level;
    }
    return Rtn;
}

void UAutoLayoutStrategy::RandomLayoutOneTree(UGraphNode *RootNode, const FBox2D &Bound)
{
    int Level                           = 0;
    TArray<UGraphNode *> CurrLevelNodes = { RootNode };
    TArray<UGraphNode *> NextLevelNodes;

    while (CurrLevelNodes.Num() != 0)
    {
        for (int i = 0; i < CurrLevelNodes.Num(); ++i)
        {
            UGraphNode *Node = CurrLevelNodes[i];
            check(Node != nullptr);

            UGraphEdGraphNode *EdNode_Node = EdGraph->NodeMap[Node];

            EdNode_Node->NodePosX = UKismetMathLibrary::RandomFloatInRange(Bound.Min.X, Bound.Max.X);
            EdNode_Node->NodePosY = UKismetMathLibrary::RandomFloatInRange(Bound.Min.Y, Bound.Max.Y);

            for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
            {
                NextLevelNodes.Add(Node->ChildrenNodes[j]);
            }
        }

        CurrLevelNodes = NextLevelNodes;
        NextLevelNodes.Reset();
        ++Level;
    }
}

int32 UAutoLayoutStrategy::GetNodeWidth(UGraphEdGraphNode *EdNode)
{
    return EdNode->SEdNode->GetCachedGeometry().GetLocalSize().X;
}

int32 UAutoLayoutStrategy::GetNodeHeight(UGraphEdGraphNode *EdNode)
{
    return EdNode->SEdNode->GetCachedGeometry().GetLocalSize().Y;
}
