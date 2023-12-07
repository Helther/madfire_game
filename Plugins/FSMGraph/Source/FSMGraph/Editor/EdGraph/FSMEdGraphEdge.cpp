#include "FSMEdGraphEdge.h"
#include "FSMEdGraphNode.h"
#include "FSMEdGraph.h"
#include "FSMGraph.h"


UFSMEdGraphEdge::UFSMEdGraphEdge() 
{
}

UFSMEdGraphEdge::~UFSMEdGraphEdge() 
{}


void UFSMEdGraphEdge::AllocateDefaultPins() 
{
	UEdGraphPin *Inputs = CreatePin(EGPD_Input, TEXT("Edge"), FName(), TEXT("In"));
	Inputs->bHidden = true;
	UEdGraphPin *Outputs = CreatePin(EGPD_Output, TEXT("Edge"), FName(), TEXT("Out"));
	Outputs->bHidden = true;
}

FText UFSMEdGraphEdge::GetNodeTitle(ENodeTitleType::Type TitleType) const 
{
	return FText::FromString("");
}

void UFSMEdGraphEdge::PinConnectionListChanged(UEdGraphPin *Pin) 
{
	if(Pin->LinkedTo.Num() == 0)
    {
		Modify();
		
		if(UEdGraph *ParentGraph = GetGraph())
        {
			ParentGraph->Modify();
        }
		DestroyNode();
	}
}

void UFSMEdGraphEdge::PrepareForCopying() 
{
	//GraphEdge->Rename(nullptr)
}

void UFSMEdGraphEdge::CreateConnections(UFSMEdGraphNode *Start, UFSMEdGraphNode *End)
{
	auto graph = CastChecked<UFSMEdGraph>(GetGraph())->GetGraphAsset();

	if(!(Start && End))
		return;	

	if(graph->Validate(Start->RealName) && graph->Validate(End->RealName))
		return;

	startName = Start->RealName;
	endName = End->RealName;

	//Pins[0]->MakeLinkTo(Start->GetOutputPin());
	//Pins[1]->MakeLinkTo(End->GetInputPin());
}

UFSMEdGraphNode *UFSMEdGraphEdge::GetStartNode()
{
	return nullptr;
}


UFSMEdGraphNode *UFSMEdGraphEdge::GetEndNode()
{
	return nullptr;
}