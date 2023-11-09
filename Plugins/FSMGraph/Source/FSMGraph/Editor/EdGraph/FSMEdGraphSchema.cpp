#include "FSMEdGraphSchema.h"
#include "FSMConnectionDrawingPolicy.h"
#include "FSMEdGraphSchemaActions_NewNode.h"
#include "../../FSMGraph.h"
#include "FSMEdGraphNode.h"

void UFSMEdGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder &ContextMenuBuilder) const
{
	FText ToolTip = FText::FromString("Add Node to the Graph");
	FText MenuDesc = FText::FromString("NodeName");

	FCategorizedGraphActionListBuilder BaseBuilder(TEXT("Base Nodes"));

	TSharedPtr<FSMEdGraphSchemaAction_NewNode> NewNodeAction;

	NewNodeAction = MakeShareable(new FSMEdGraphSchemaAction_NewNode(FText::FromString("AddNewNode"),FText::FromString("TEST2"),FText::FromString("TEST3"), 0));

	BaseBuilder.AddAction(NewNodeAction);

	ContextMenuBuilder.Append(BaseBuilder);
}

void UFSMEdGraphSchema::GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	
}

const FPinConnectionResponse UFSMEdGraphSchema::CanCreateConnection(const UEdGraphPin *A, const UEdGraphPin *B) const
{
	if(!(A && B))
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both pins must be available."));

	if(A->GetOwningNode() == B->GetOwningNode())
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You cannot connect a node to itself."));

	if(A->Direction == EGPD_Input && B->Direction == EGPD_Input)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You cannot connect an input pin to another input pin"));

	if(A->Direction == EGPD_Output && B->Direction == EGPD_Output)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You cannot connect an output pin to another output pin"));

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

FConnectionDrawingPolicy *UFSMEdGraphSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect &InClippingRect, class FSlateWindowElementList &InDrawElements, class UEdGraph *InGraphObj) const
{
	return new FSMConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

void UFSMEdGraphSchema::CreateDefaultNodesForGraph(UEdGraph &Graph) const 
{
	if(Graph.Nodes.Num() == 0)
    {
		UFSMGraph *GraphAsset = Cast<UFSMGraph>(Graph.GetOuter());

		GraphAsset->Modify();
		Graph.Modify();

		
		FGraphNodeCreator<UFSMEdGraphNode> Creator(Graph);
		UFSMEdGraphNode *EdNode = Creator.CreateNode();
		//Set Node Info
		EdNode->AllocateDefaultPins();

		Creator.Finalize();
		EdNode->NodePosX = 0;
		EdNode->NodePosY = 0;
	}
}