#include "GraphNode.h"
#include "Graph.h"

UGraphNode::UGraphNode()
{
#if WITH_EDITORONLY_DATA
    CompatibleGraphType = UGraph::StaticClass();
    BackgroundColor     = FLinearColor::Black;
#endif
}

UGraphNode::~UGraphNode()
{}

UGraphEdge* UGraphNode::GetEdge(UGraphNode* ChildNode)
{
    return Edges.Contains(ChildNode) ? Edges.FindChecked(ChildNode) : nullptr;
}

bool UGraphNode::IsLeafNode() const 
{
    return false;
}

UGraph* UGraphNode::GetGraph() const
{
    return Graph;
}

FText UGraphNode::GetDescription_Implementation() const 
{
    return FText();
}


#if WITH_EDITOR

FLinearColor UGraphNode::GetBackgroundColor() const
{
    return BackgroundColor;
}

FText UGraphNode::GetNodeTitle() const
{
    return NodeTitle.IsEmpty() ? GetDescription() : NodeTitle;
}

void UGraphNode::SetNodeTitle(const FText& NewTitle)
{
     NodeTitle = NewTitle;
}

bool UGraphNode::CanCreateConnection(UGraphNode* Other, FText& ErrorMessage)
{
    return true;
}
#endif