#include "FSMEdGraphSchemaActions_NewEdge.h"
#include "FSMEdGraphEdge.h"
#include "FSMGraph.h"

FSMEdGraphSchemaAction_NewEdge::FSMEdGraphSchemaAction_NewEdge()
{

}

FSMEdGraphSchemaAction_NewEdge::FSMEdGraphSchemaAction_NewEdge(FText, FText, FText, const int32)
{

}

UEdGraphNode *FSMEdGraphSchemaAction_NewEdge::PerformAction(class UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectedNewNode) 
{
	check(ParentGraph);

	ParentGraph->Modify();

	UFSMGraph *GraphAsset = CastChecked<UFSMGraph>(ParentGraph->GetOuter());
	FName NodeName = GraphAsset->SpawnNode();
	GraphAsset->Modify();

	UEdGraphNode *EditorEdge = CreateEditorNode(ParentGraph, false, NodeName);
		
	EditorEdge->AutowireNewNode(FromPin);
	EditorEdge->NodePosX = Location.X;
	EditorEdge->NodePosY = Location.Y;

	return EditorEdge;
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