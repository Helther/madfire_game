#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "FSMEdGraphSchema.generated.h"


USTRUCT()
struct FFSMGraphSchemaAction_NewStateNode : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	TObjectPtr<UEdGraphNode> NodeTemplate;

	FFSMGraphSchemaAction_NewStateNode() :
		FEdGraphSchemaAction(),
		NodeTemplate(nullptr)
    {}

	FFSMGraphSchemaAction_NewStateNode(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping) :
		FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping),
		NodeTemplate(nullptr)
    {}

	UEdGraphNode *PerformAction(class UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	void AddReferencedObjects(FReferenceCollector &Collector) override;

	template<typename NodeType>
	static NodeType *SpawnNodeFromTemplate(class UEdGraph *ParentGraph, NodeType *InTemplateNode, const FVector2D Location = FVector2D(0.f,0.f), bool bSelectNewNode = true)
    {
		FFSMGraphSchemaAction_NewStateNode Action;
		Action.NodeTemplate = InTemplateNode;
		return Cast<NodeType>(Action.PerformAction(ParentGraph, nullptr, Location, bSelectedNewNode));
	}
};

USTRUCT()
struct FFSMGraphSchemaAction_NewComment : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	FFSMGraphSchemaAction_NewComment() : 
		FEdGraphSchemaAction() 
	{}

	FFSMGraphSchemaAction_NewComment(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping) : 
		FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)   
	{}

	UEdGraphNode *PerformAction(class UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
};

UCLASS()
class UFSMEdGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	//void GetGraphContextActions(FGraphContextMenuBuilder &ContextMenuBuilder) const override;	
	//const FPinConnectionResponse CanCreateConnection(const UEdGraphPin *A, const UEdGraphPin *B) const override;
	//void CreateDefaultNodesForGraph(UEdGraph &Graph) const override;
	const FPinConnectionResponse CanCreateConnection(const UEdGraphPin *A, const UEdGraphPin *B) const override;	
	bool TryCreateConnection(UEdGraphPin *A, UEdGraphPin *B) const;
	bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin *PinA, UEdGraphPin *PinB) const override;
	bool TryRelinkConnectionTarget(UEdGraphPin *SourcePin, UEdGraphPin *OldTargetPin, UEdGraphPin *NewTargetPin, const TArray<UEdGraphNode*> &InSelectedGraphNodes) const override;
	bool IsConnectionRelinkingAllowed(UEdGraphPin *InPin) const override;
	const FPinConnectionResponse CanRelinkConnectionToPin(const UEdGraphPin *OldSourcePin, const UEdGraphPin *TargetPinCandidate) const override;
	void GetContextMenuActions(class UToolMenu *Menu, class UGraphNodeContextMenuContext *Context) const override;
	EGraphType GetGraphType(const UEdGraph *TestEdGraph) const override;
	void GetGraphContextActions(FGraphContextMenuBuilder &ContextMenuBuilder) const override;
	//FLinearColor GetPinTypeColor(const FEdGraphPinType &PinType) const override;
	//void GetGraphDisplayInformation(const UEdGraph &Graph, FGraphDisplayInfo &DisplayInfo) const override;
	//void DroppedAssetsOnGraph(const TArray<FAssetData> &Assets, const FVector2D &GraphPosition, UEdGraph *Graph) const override;
	//void DroppedAssetsOnNode(const TArray<FAssetData> &Assets, const FVector2D &GraphPosition, UEdGraphNode *Node) const override;
	//void DroppedAssetsOnPin(const TArray<FAssetData> &Assets, const FVector2D &GraphPosition, UEdGraphPin *Pin) const override;
	//void GetAssetsNodeHoverMessage(const TArray<FAssetData> &Assets, const UEdGraphNode *HoverNode, FString &OutTooltipText, bool &OutOkIcon) const override;
	void GetAssetsPinHoverMessage(const TArray<FAssetData> &Assets, const UEdGraphPin *HoverNode, FString &OutTooltipText, bool &OutOkIcon) const override;
	class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;
	
	//void BreakNodeLinks(UEdGraphNode &TargetNode) const override;
	//void BreakPinLinks(UEdGraphPin &TargetPin, bool bSendsNodeNotification) const override;
	//void BreakSinglePinLink(UEdGraphPin *SourcePin, UEdGraphPin *TargetPin) const override;


	/*
	void CreateDefaultNodesForGraph(UEdGraph &Graph) const override;
	void GetGraphDisplayInformation(const UEdGraph &Graph, FGraphDisplayInfo &DisplayInfo) const override;
	bool CanDuplicateGraph(UEdGraph *InSourceGraph) const override;*/
};