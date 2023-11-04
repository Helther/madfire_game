#pragma once

#include "CoreMinimal.h"
#include "../../Graph.h"
#include "../../GraphEdge.h"
#include "../../GraphNode.h"
#include "AssetGraphSchema.generated.h"

class UGraphEdGraphNode;
class UGraphEdGraphEdge;
class UAutoLayoutStrategy;


USTRUCT()
struct FAssetGraphSchemaAction_NewNode : public FEdGraphSchemaAction
{
    GENERATED_BODY()

public:
    FAssetGraphSchemaAction_NewNode() : 
		NodeTemplate(nullptr) {}

	FAssetGraphSchemaAction_NewNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping):
		FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), 
		NodeTemplate(nullptr) {}

	UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	void AddReferencedObjects(FReferenceCollector& Collector) override;

	UGraphEdGraphNode *NodeTemplate;
};

USTRUCT()
struct FAssetGraphSchemaAction_NewEdge : public FEdGraphSchemaAction
{
    GENERATED_BODY()
public:
    FAssetGraphSchemaAction_NewEdge() : 
        NodeTemplate(nullptr) 
    {}

    FAssetGraphSchemaAction_NewEdge(const FText &InNodeCategory, const FText &InMenuDesc, const FText &InToolTip, const int32 InGrouping) :
        FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping),
        NodeTemplate(nullptr)
    {}

    virtual UEdGraphNode *PerformAction(class UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
    virtual void AddReferencedObjects(FReferenceCollector &Collector) override;

    UGraphEdGraphEdge *NodeTemplate;
};

UCLASS(MinimalAPI)
class UAssetGraphSchema : public UEdGraphSchema
{
    GENERATED_BODY()

public:
    void GetBreakLinkToSubMenuActions(class UToolMenu* Menu, class UEdGraphPin* InGraphPin);
	void GetBreakLinkToSubMenuActions(class FMenuBuilder& MenuBuilder, class UEdGraphPin* InGraphPin);
	EGraphType GetGraphType(const UEdGraph* TestEdGraph) const override;
 	void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
    void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;    
    const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
    bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const override;
    class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;
    FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
    void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
    void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;
    void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;
    UEdGraphPin* DropPinOnNode(UEdGraphNode* InTargetNode, const FName& InSourcePinName, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection) const override;
    bool SupportsDropPinOnNode(UEdGraphNode* InTargetNode, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection, FText& OutErrorMessage) const override;
    bool IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const override;
    int32 GetCurrentVisualizationCacheID() const override;
    void ForceVisualizationCacheClear() const override;

private:
	static int32 CurrentCacheRefreshID;

};