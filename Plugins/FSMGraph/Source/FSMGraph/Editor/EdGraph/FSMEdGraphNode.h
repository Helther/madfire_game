#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "SubclassOf.h"

#include "FSMEdGraphNode.generated.h"

class UFSMGraph;
class UFSMEdGraphEdge;

UCLASS(MinimalAPI)
class UFSMEdGraphNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	//Interface from UEdGraphNode
	TSharedPtr<SGraphNode> CreateVisualWidget() override;
	void AllocateDefaultPins() override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	void PrepareForCopying() override;
	void DestroyNode() override;
	void AutowireNewNode(UEdGraphPin *FromPin) override;
	void SetAssetNode(FName index);
	FName GetAssetNode() const;
	virtual bool RenameUniqueNode(const FName &NewName);

	virtual UEdGraphPin *GetOutputPin() const;
	virtual UEdGraphPin *GetInputPin() const;

	UPROPERTY(EditDefaultsOnly)
	FName AssetNode;

	UPROPERTY(EditDefaultsOnly)
	bool IsInit;

	void GetTransitionList(TArray<UFSMEdGraphEdge*> &OutTransitions, bool bWantSorted) const;


protected:
	TSharedPtr<SGraphNode> SlateNode;
	
};