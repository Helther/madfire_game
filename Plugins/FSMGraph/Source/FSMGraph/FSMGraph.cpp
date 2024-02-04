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

FSMState &UFSMGraph::GetState(const FName &NodeName)
{
	return StatesMap[NodeName];
}

void UFSMGraph::AddState(const FName &NodeName, FSMState &state)
{
	StatesMap.Add(NodeName, state);
}

void UFSMGraph::RemoveState(const FName &StateName) 
{
	StatesMap.Remove(StateName);
	
	for(auto &State : StatesMap)
    {
		State.Value.Links.RemoveAll([StateName](const FSMLink &Link) {return Link.LinkedStateID == StateName;});		
    }
}

bool UFSMGraph::Validate(const FName &NodeName)
{
	return !StatesMap.Contains(NodeName);
}