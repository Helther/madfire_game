#include "FSMEdGraphSchema.h"
#include "FSMConnectionDrawingPolicy.h"
#include "FSMEdGraphSchemaActions_NewNode.h"
#include "FSMEdGraphSchemaActions_NewEdge.h"
#include "../../FSMGraph.h"
#include "FSMEdGraphNode.h"
#include "FSMEdGraphEdge.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "ToolMenuSection.h"
#include "ToolMenus.h"

/*
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
*/

TSharedPtr<FSMEdGraphSchemaAction_NewNode> AddNewStateNodeAction(FGraphContextMenuBuilder &ContextMenuBuilder, const FText &Category, const FText &MenuDesc, const FText &Tooltip, const int32 Grouping = 0)
{
	TSharedPtr<FSMEdGraphSchemaAction_NewNode> NewNode(new FSMEdGraphSchemaAction_NewNode(Category, MenuDesc, Tooltip, Grouping));
	ContextMenuBuilder.AddAction(NewNode);
	return NewNode;
}

UEdGraphNode *FFSMGraphSchemaAction_NewStateNode::PerformAction(UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode *ResultNode = nullptr;

	if(NodeTemplate != nullptr)
    {
		const FScopedTransaction Transaction(FText::FromString("Test"));

		ParentGraph->Modify();

		if(FromPin)
			FromPin->Modify();

		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		NodeTemplate->NodePosX = static_cast<int32>(Location.X);
		NodeTemplate->NodePosY = static_cast<int32>(Location.Y);
		//NodeTemplate->SnapToGrid(GetDefault<UEditorStyle)
		ResultNode = NodeTemplate;

		ResultNode->SetFlags(RF_Transactional);

		//UBlueprint *Blueprint = FBlueprintEditorUtils::FindBlueprintForGraphChecked(ParentGraph);
		//FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    }


    return ResultNode;
}

void FFSMGraphSchemaAction_NewStateNode::AddReferencedObjects(FReferenceCollector &Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}


UEdGraphNode *FFSMGraphSchemaAction_NewComment::PerformAction(UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode_Comment *CommentTemplate = NewObject<UEdGraphNode_Comment>();

	UBlueprint *Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(ParentGraph);
	FVector2D SpawnLocation = Location;

	FSlateRect Bounds;
	if((Blueprint != nullptr) && FKismetEditorUtilities::GetBoundsForSelectedNodes(Blueprint, Bounds, 50.f))
    {
		CommentTemplate->SetBounds(Bounds);
		SpawnLocation.X = CommentTemplate->NodePosX;
		SpawnLocation.Y = CommentTemplate->NodePosY;
    }

    return FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation);
}
/*
void UFSMEdGraphSchema::CreateDefaultNodesForGraph(UEdGraph &Graph) const 
{

}*/

const FPinConnectionResponse UFSMEdGraphSchema::CanCreateConnection(const UEdGraphPin *A, const UEdGraphPin *B) const
{
	if(A->GetOwningNode() == B->GetOwningNode())
    {
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both are on the same node"));
    }

	const bool bPinAIsEntry = A->GetOwningNode()->IsA(UFSMEdGraphNode::StaticClass());
	const bool bPinBIsEntry = B->GetOwningNode()->IsA(UFSMEdGraphNode::StaticClass());
	const bool bPinAIsStateNode = A->GetOwningNode()->IsA(UFSMEdGraphNode::StaticClass());
	const bool bPinBIsStateNode = B->GetOwningNode()->IsA(UFSMEdGraphNode::StaticClass());

	if(bPinAIsEntry || bPinBIsEntry)
    {    
		if(bPinAIsEntry && bPinBIsStateNode)
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, TEXT(""));
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Entry must connect to a state node"));
	}

	const bool bPinAIsTransition = A->GetOwningNode()->IsA(UFSMEdGraphEdge::StaticClass());
	const bool bPinBIsTransition = B->GetOwningNode()->IsA(UFSMEdGraphEdge::StaticClass());

	if(bPinAIsTransition && bPinAIsTransition)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot wire a transition to a transition"));

	bool bDirectionsOk = false;

	if((A->Direction == EGPD_Input) && (B->Direction == EGPD_Output))
		bDirectionsOk = true;
    else if((B->Direction == EGPD_Input) && (A->Direction == EGPD_Output))
		bDirectionsOk = true;

	if(bPinAIsTransition)
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, TEXT(""));
    else if(bPinBIsTransition)
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, TEXT(""));
    else if(!bPinAIsTransition && !bPinBIsTransition)
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, TEXT("Create a transition"));
    else
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));

    return FPinConnectionResponse();
}

bool UFSMEdGraphSchema::TryCreateConnection(UEdGraphPin *A, UEdGraphPin *B) const
{
	if(B->Direction == A->Direction)
    {
		CreateAutomaticConversionNodeAndConnections(A,B);
		/*if(UFSMEdGraphNode *Node = Cast<UFSMEdGraphNode>(B->GetOwningNode()))
        {
			if(A->Direction == EGPD_Input)
				B = Node->GetOutputPin();
            else
				B = Node->GetInputPin();
		}*/
    }

	const bool bModified = UEdGraphSchema::TryCreateConnection(A,B);

	if(bModified)
    {
		//UBlueprint *Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(A->GetOwningNode());
		//FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    }

    return bModified;
}

bool UFSMEdGraphSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin *PinA, UEdGraphPin *PinB) const
{
	UFSMEdGraphNode *NodeA = Cast<UFSMEdGraphNode>(PinA->GetOwningNode());
	UFSMEdGraphNode *NodeB = Cast<UFSMEdGraphNode>(PinB->GetOwningNode());

	if((NodeA != nullptr) && (NodeB != nullptr) 
		&& (NodeA->GetInputPin() != nullptr) && (NodeA->GetOutputPin() != nullptr)
		&& (NodeB->GetInputPin() != nullptr) && (NodeB->GetOutputPin() != nullptr))
    {
		UFSMEdGraphEdge *TransitionNode = FSMEdGraphSchemaAction_NewEdge::SpawnNodeFromTemplate<UFSMEdGraphEdge>(NodeA->GetGraph(), NewObject<UFSMEdGraphEdge>(), FVector2D(0.f,0.f), false);

		if(PinA->Direction == EGPD_Output)
        {
			TransitionNode->CreateConnections(NodeA, NodeB);
		}
        else
        {
			TransitionNode->CreateConnections(NodeB, NodeA);
        }
		//UBlueprint *Blueprint = FBlueprintEditorUtils::FindBlueprintForGraphChecked(TransitionNode->GEtBou);
		//FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		return true;
    }

    return false;
}

bool UFSMEdGraphSchema::TryRelinkConnectionTarget(UEdGraphPin *SourcePin, UEdGraphPin *OldTargetPin, UEdGraphPin *NewTargetPin, const TArray<UEdGraphNode *> &InSelectedGraphNodes) const
{
	const FPinConnectionResponse Response = CanCreateConnection(SourcePin, NewTargetPin);

	if(Response.Response == ECanCreateConnectionResponse::CONNECT_RESPONSE_DISALLOW)
		return false;

	UFSMEdGraphNode *OldNode = Cast<UFSMEdGraphNode>(OldTargetPin->GetOwningNode());
	UFSMEdGraphNode *NewNode = Cast<UFSMEdGraphNode>(NewTargetPin->GetOwningNode());

	if(OldNode == nullptr || OldNode->GetInputPin() == nullptr || OldNode->GetOutputPin() == nullptr ||
	   NewNode == nullptr || NewNode->GetInputPin() == nullptr || NewNode->GetOutputPin() == nullptr)
		return false;

	//const TArray<UFSMEdGraphNode*> TransitionNodes = UFSMEdGraphEdge::GetListTran

    return true;
}

bool UFSMEdGraphSchema::IsConnectionRelinkingAllowed(UEdGraphPin *InPin) const
{
	if(InPin && InPin->GetOwningNode())
    {
		return true;
    }

    return false;
}

const FPinConnectionResponse  UFSMEdGraphSchema::CanRelinkConnectionToPin(const UEdGraphPin *OldSourcePin, const UEdGraphPin *TargetPinCandidate) const
{
	FPinConnectionResponse Response = CanCreateConnection(OldSourcePin, TargetPinCandidate);
	if(Response.Response != CONNECT_RESPONSE_DISALLOW)
		Response.Message = FText::FromString("Relink transition");

    return Response;
}

FConnectionDrawingPolicy *UFSMEdGraphSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect &InClippingRect, class FSlateWindowElementList &InDrawElements, class UEdGraph *InGraphObj) const
{
	return new FSMConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}
/*
void UFSMEdGraphSchema::BreakNodeLinks(UEdGraphNode &TargetNode) const 
{

}

void UFSMEdGraphSchema::BreakPinLinks(UEdGraphPin &TargetPin, bool bSendsNodeNotification) const
{

}

void UFSMEdGraphSchema::BreakSinglePinLink(UEdGraphPin *SourcePin, UEdGraphPin *TargetPin) const
{

}
*/
void UFSMEdGraphSchema::GetContextMenuActions(UToolMenu *Menu, UGraphNodeContextMenuContext *Context) const
{
	check(Context && Context->Graph);
	UBlueprint *OwnerBlueprint = FBlueprintEditorUtils::FindBlueprintForGraphChecked(Context->Graph);

	if(Context->Node)
    {
        {
			//FToolMenuSection &Section = Menu->AddSection("StateMachineNodeActions", FText::FromString("NodeActions"));
			if(!Context->bIsDebugging)
            {
				//Section.AddMenuEntry(FGenericCommands::Get().Delete);
				//Section.AddMenuEntry(FGenericCommands::Get().Cut);
				//Section.AddMenuEntry(FGenericCommands::Get().Copy);
				//Section.AddMenuEntry(FGenericCommands::Get().Duplicate);
				//Section.AddMenuEntry(FGraphEditorCommands::Get().ReconstructNodes);
				//Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
				//if(Context->Node->bCanRenameNode)
					//Section.AddMenuEntry(FGenericCommands::Get().Rename);
			}
		}
    }
	Super::GetContextMenuActions(Menu, Context);
}

EGraphType UFSMEdGraphSchema::GetGraphType(const UEdGraph *TestEdGraph) const
{
    return GT_StateMachine;
}

void UFSMEdGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder &ContextMenuBuilder) const
{	
	TSharedPtr<FSMEdGraphSchemaAction_NewNode> NewNodeAction;
	NewNodeAction = MakeShareable(new FSMEdGraphSchemaAction_NewNode(FText::GetEmpty(),FText::FromString("AddNewNode"),FText::FromString("TooltipText"), 0));

	ContextMenuBuilder.AddAction(NewNodeAction);
	

	if(!ContextMenuBuilder.FromPin)
    {
		//UBlueprint *OwnerBlueprint = FBlueprintEditorUtils::FindBlueprintForGraphChecked(ContextMenuBuilder.CurrentGraph);
		//const bool bIsManyNodesSelected = (FKismetEditorUtilities::GetNumberOfSelectedNodes(OwnerBlueprint) > 0);
		//const FText MenuDescription = bIsManyNodesSelected ? FText::FromString("") : FText::FromString("");
		const FText MenuDescription = FText::FromString("AddComment");
		const FText ToolTip = FText::FromString("");

		TSharedPtr<FFSMGraphSchemaAction_NewComment> NewComment(new FFSMGraphSchemaAction_NewComment(FText::GetEmpty(), MenuDescription, ToolTip, 0));
		ContextMenuBuilder.AddAction(NewComment);
    }
}
/*
FLinearColor UFSMEdGraphSchema::GetPinTypeColor(const FEdGraphPinType &PinType) const
{
    return FLinearColor();
}

void UFSMEdGraphSchema::GetGraphDisplayInformation(const UEdGraph &Graph, FGraphDisplayInfo &DisplayInfo) const
{

}

void UFSMEdGraphSchema::DroppedAssetsOnGraph(const TArray<FAssetData> &Assets, const FVector2D &GraphPosition, UEdGraph *Graph) const
{

}

void UFSMEdGraphSchema::DroppedAssetsOnNode(const TArray<FAssetData> &Assets, const FVector2D &GraphPosition,  UEdGraphNode *Node) const
{

}

void UFSMEdGraphSchema::DroppedAssetsOnPin(const TArray<FAssetData> &Assets, const FVector2D &GraphPosition, UEdGraphPin *Pin) const
{

}

void UFSMEdGraphSchema::GetAssetsNodeHoverMessage(const TArray<FAssetData> &Assets, const UEdGraphNode *HoverNode, FString &OutTooltipText, bool &OutOkIcon) const
{
	
}*/

void UFSMEdGraphSchema::GetAssetsPinHoverMessage(const TArray<FAssetData> &Assets, const UEdGraphPin *HoverNode, FString &OutTooltipText, bool &OutOkIcon) const
{
	OutTooltipText = TEXT("");
	OutOkIcon = false;
}
/*
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

void UFSMEdGraphSchema::GetGraphDisplayInformation(const UEdGraph &Graph, FGraphDisplayInfo &DisplayInfo) const
{
	DisplayInfo.PlainName = FText::FromString(Graph.GetName());

}

bool UFSMEdGraphSchema::CanDuplicateGraph(UEdGraph *InSourceGraph) const 
{
	return false; 

}
*/

