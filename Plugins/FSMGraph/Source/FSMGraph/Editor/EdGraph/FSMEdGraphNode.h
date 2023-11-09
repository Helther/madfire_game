#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "SubclassOf.h"

#include "FSMEdGraphNode.generated.h"

class UFSMGraph;

UCLASS(MinimalAPI)
class UFSMEdGraphNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UFSMEdGraphNode();
	~UFSMEdGraphNode() override;

	//Interface from UEdGraphNode
	TSharedPtr<SGraphNode> CreateVisualWidget() override;
	void AllocateDefaultPins() override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	void PrepareForCopying() override;
	void DestroyNode() override;
	void AutowireNewNode(UEdGraphPin *FromPin) override;


	void SetAssetNode(UFSMGraph *graph, int32 index);
	TPair<UFSMGraph *, int32> GetAssetNode() const;


	void PostCopyNode();
	bool RenameUniqueNode(const FText &NewName);
	FText GetEdNodeName() const;
	void SetEdNodeName(const FText &Name);
	void SetEdNodeName(const FName &Name);
	TSharedPtr<SWidget> GetContentWidget();
	void UpdateVisualNode();
	void SaveNodesAsChildren(TArray<UEdGraphNode*> &Children);

	TPair<UFSMGraph *, int32> AssetNode;

protected:
	bool HasOutputPins();
	bool HasInputPins();
	TSharedPtr<SGraphNode> SlateNode;
	
	UPROPERTY()
	FText EdNodeName;
};