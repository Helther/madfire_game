#include "FSMEdGraphSchemaActions_NewNode.h"
#include "FSMGraph.h"
#include "FSMEdGraphNode.h"

FSMEdGraphSchemaAction_NewNode::FSMEdGraphSchemaAction_NewNode() :
	FEdGraphSchemaAction()
{}


FSMEdGraphSchemaAction_NewNode::FSMEdGraphSchemaAction_NewNode(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping) :
	FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping)
{}

UEdGraphNode *FSMEdGraphSchemaAction_NewNode::PerformAction(class UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectNewNode) 
{
	check(ParentGraph);

	ParentGraph->Modify();

	UFSMGraph *GraphAsset = CastChecked<UFSMGraph>(ParentGraph->GetOuter());
	GraphAsset->Modify();

	UEdGraphNode *EditorNode = CreateEditorNode(ParentGraph, bSelectNewNode);

	EditorNode->AutowireNewNode(FromPin);
	EditorNode->NodePosX = Location.X;
	EditorNode->NodePosY = Location.Y;

	return EditorNode;
}

UEdGraphNode *FSMEdGraphSchemaAction_NewNode::CreateEditorNode(UEdGraph *ParentGraph, bool bSelectNewNode)
{
	FGraphNodeCreator<UFSMEdGraphNode> Creator(*ParentGraph);

	UFSMEdGraphNode *EdNode = Creator.CreateNode(bSelectNewNode);
	//EdNode->SetAssetNode(AssetNode);
	Creator.Finalize();

	return EdNode;
}