#include "FSMEdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "FSMGraph.h"

bool UFSMEdGraph::IsNameUnique(const FText &InName) 
{
	bool bUnique = true;
	for(UEdGraphNode *Node : Nodes)
    {

    
	}
	return bUnique;
}

void UFSMEdGraph::SaveGraph()
{
	LinkAssetNodes();
	MapNamedNodes();
}

void UFSMEdGraph::ClearOldLinks()
{

}

void UFSMEdGraph::LinkAssetNodes()
{

}

void UFSMEdGraph::RefreshNodes()
{

}

void UFSMEdGraph::MapNamedNodes()
{

}

UFSMGraph *UFSMEdGraph::GetGraphAsset()
{
	return Cast<UFSMGraph>(GetOuter());
}