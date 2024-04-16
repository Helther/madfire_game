#include "CondGraphSchema.h"
#include "Nodes/CondGraphResult.h"
#include "CondGraph.h"
#include "../Editor/EdGraph/FSMEdGraphEdge.h"
#include "EdGraphSchema_K2_Actions.h"

void UFSMCondGraphSchema::CreateDefaultNodesForGraph(UEdGraph &Graph) const 
{
	FGraphNodeCreator<UFSMConditionResultNode> NodeCreator(Graph);
	UFSMConditionResultNode *ResultNode = NodeCreator.CreateNode();

	NodeCreator.Finalize();
	SetNodeMetaData(ResultNode, FNodeMetadata::DefaultGraphNode);

	UFSMCondGraph *CondGraph = CastChecked<UFSMCondGraph>(&Graph);
	CondGraph->MyResultNode = ResultNode;
}

bool UFSMCondGraphSchema::CanDuplicateGraph(UEdGraph *InSourceGraph) const { return false; }

void UFSMCondGraphSchema::GetGraphDisplayInformation(const UEdGraph &Graph, FGraphDisplayInfo &DisplayInfo) const
{
	DisplayInfo.PlainName = FText::FromString(Graph.GetName());

	const UFSMEdGraphEdge *Edge = Cast<UFSMEdGraphEdge>(Graph.GetOuter());

	if(Edge == nullptr && IsValid(&Graph))
    {
		//TODO: Something
    }

	if(Edge)
    {
		FFormatNamedArguments Args;
		Args.Add(TEXT("NodeTitle"), Edge->GetNodeTitle(ENodeTitleType::FullTitle));
		DisplayInfo.PlainName = FText::FromString("NAME");
		DisplayInfo.Tooltip = FText::FromString("ToolTip");
    }
	DisplayInfo.DisplayName = DisplayInfo.PlainName;
}

bool UFSMCondGraphSchema::ShouldAlwaysPurgeOnModification() const { return true; }

void UFSMCondGraphSchema::HandleGraphBeingDeleted(UEdGraph &GraphBeingRemoved) const 
{

}

void UFSMCondGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder &ContextMenuBuilder) const
{

}

bool UFSMCondGraphSchema::DoesSupportCollapsedNodes() const { return false; }

bool UFSMCondGraphSchema::DoesSupportEventDispatcher() const { return false; }
