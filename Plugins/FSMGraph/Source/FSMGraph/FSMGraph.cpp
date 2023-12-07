#include "FSMGraph.h"
#include "EdGraphUtilities.h"

FName UFSMGraph::SpawnNode()
{
	//MakeUniqueObjectName()
	int UniqueNameIDCounter = 0;	
	FName NodeName;
	do
    {
		 FString StringName = "State" + FString::FromInt(UniqueNameIDCounter++);
		 NodeName = FName(*StringName);
    }
	while(!Validate(NodeName)); 
	
	FSMState state;
	StatesMap.Add(NodeName, state);

	return NodeName;
}

FSMState &UFSMGraph::GetState(FName &NodeName)
{
	return StatesMap[NodeName];
}

void UFSMGraph::AddState(FName &NodeName, FSMState &state)
{
	StatesMap.Add(NodeName, state);
}

void UFSMGraph::RemoveState(FName &StateName) 
{
	StatesMap.Remove(StateName);
}

bool UFSMGraph::Validate(FName &NodeName)
{
	return !StatesMap.Contains(NodeName);
}