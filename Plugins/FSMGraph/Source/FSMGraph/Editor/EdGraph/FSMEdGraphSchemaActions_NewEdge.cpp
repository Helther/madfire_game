#include "FSMEdGraphSchemaActions_NewEdge.h"
#include "FSMEdGraphEdge.h"
#include "FSMGraph.h"

FSMEdGraphSchemaAction_NewEdge::FSMEdGraphSchemaAction_NewEdge() :
	FEdGraphSchemaAction(),
	NodeTemplate(nullptr)
{}

FSMEdGraphSchemaAction_NewEdge::FSMEdGraphSchemaAction_NewEdge(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping) : 
	FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping),
	NodeTemplate(nullptr)
{}

UEdGraphNode *FSMEdGraphSchemaAction_NewEdge::PerformAction(class UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectedNewNode) 
{
	UEdGraphNode *ResultNode = nullptr;

	if(NodeTemplate != nullptr)
    {
		const FScopedTransaction Transaction(FText::FromString("Test"));

		ParentGraph->Modify();

		if(FromPin)
			FromPin->Modify();

		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectedNewNode);

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

	/*
	check(ParentGraph);

	ParentGraph->Modify();

	UFSMGraph *GraphAsset = CastChecked<UFSMGraph>(ParentGraph->GetOuter());
	FName NodeName = GraphAsset->SpawnNode();
	GraphAsset->Modify();

	UEdGraphNode *EditorEdge = CreateEditorNode(ParentGraph, false, NodeName);
		
	EditorEdge->AutowireNewNode(FromPin);
	EditorEdge->NodePosX = Location.X;
	EditorEdge->NodePosY = Location.Y;

	return EditorEdge;*/
}

void FSMEdGraphSchemaAction_NewEdge::AddReferencedObjects(FReferenceCollector &Collector)
{
}

UEdGraphNode *FSMEdGraphSchemaAction_NewEdge::CreateEditorNode(UEdGraph *ParentGraph, bool bSelectNewNode, FName NodeName)
{
	FGraphNodeCreator<UFSMEdGraphEdge> Creator(*ParentGraph);

	UFSMEdGraphEdge *EdEdge = Creator.CreateNode(bSelectNewNode);
	//EdEdge->SetAssetNode(NodeName);
	Creator.Finalize();

	return EdEdge;
}