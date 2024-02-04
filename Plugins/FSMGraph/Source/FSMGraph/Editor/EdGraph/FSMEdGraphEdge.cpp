#include "FSMEdGraphEdge.h"
#include "FSMEdGraphNode.h"
#include "FSMEdGraph.h"
#include "FSMGraph.h"
#include "../SlateGraph/SFSMGraphEdge.h"
#include "../../FSMGraph.h"

UFSMEdGraphEdge::UFSMEdGraphEdge() 
{
}

UFSMEdGraphEdge::~UFSMEdGraphEdge() 
{}

TSharedPtr<SGraphNode> UFSMEdGraphEdge::CreateVisualWidget()
{	
	SlateNode = SNew(SFSMGraphEdge, this);
    return SlateNode;
}

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

void UFSMEdGraphEdge::CreateConnections(UFSMEdGraphNode *PrevState, UFSMEdGraphNode *NextState)
{
	Pins[0]->Modify();
	Pins[0]->LinkedTo.Empty();
	PrevState->GetOutputPin()->Modify();
	Pins[0]->MakeLinkTo(PrevState->GetOutputPin());

	Pins[1]->Modify();
	Pins[1]->LinkedTo.Empty();
	NextState->GetInputPin()->Modify();
	Pins[1]->MakeLinkTo(NextState->GetInputPin());

	UFSMEdGraph *Graph = Cast<UFSMEdGraph>(GetGraph());
	UFSMGraph *GraphAsset = Graph->GetGraphAsset();

	FName Prev = PrevState->AssetNode;
	FName Next = NextState->AssetNode;
	if(!GraphAsset->Validate(Prev) && !GraphAsset->Validate(Next))
    {
		GraphAsset->StatesMap[Prev].Links.Add({Next,FSMCondition()});
    }
}

UFSMEdGraphNode *UFSMEdGraphEdge::GetPreviousState() const
{
	if(Pins[0]->LinkedTo.Num() > 0)
		return Cast<UFSMEdGraphNode>(Pins[0]->LinkedTo[0]->GetOwningNode());
    else
		return nullptr;
}


UFSMEdGraphNode *UFSMEdGraphEdge::GetNextState() const
{
	if(Pins[1]->LinkedTo.Num() > 0)
		return Cast<UFSMEdGraphNode>(Pins[1]->LinkedTo[0]->GetOwningNode());
    else
		return nullptr;
}