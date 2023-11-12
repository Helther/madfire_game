#include "FSMGraph.h"
#include "EdGraphUtilities.h"

FName UFSMGraph::SpawnNode()
{
	//MakeUniqueObjectName()
	int UniqueNameIDCounter = 0;
	FName NodeName;
	do
    {
		 NodeName = "State" + UniqueNameIDCounter++;
    }
	while(!Validate(NodeName)); 
	
	FSMState state;
	StatesMap[NodeName] = state;

	return NodeName;
}

void UFSMGraph::AddState(FName &NodeName)
{
	FSMState state;
	StatesMap[NodeName] = state;
}

bool UFSMGraph::Validate(FName &NodeName)
{
	return !StatesMap.Contains(NodeName);
}