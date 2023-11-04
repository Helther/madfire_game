#pragma once

#include "CoreMinimal.h"
#include "AutoLayoutStrategy.h"
#include "TreeLayoutStrategy.generated.h"

UCLASS()
class UTreeLayoutStrategy : public UAutoLayoutStrategy
{
	GENERATED_BODY()
public:
	UTreeLayoutStrategy();
	virtual ~UTreeLayoutStrategy();

	virtual void Layout(UEdGraph* EdGraph) override;

protected:
	void InitPass(UGraphNode* RootNode, const FVector2D& Anchor);
	bool ResolveConflictPass(UGraphNode* Node);

	bool ResolveConflict(UGraphNode* LRoot, UGraphNode* RRoot);

	void GetLeftContour(UGraphNode* RootNode, int32 Level, TArray<UGraphEdGraphNode*>& Contour);
	void GetRightContour(UGraphNode* RootNode, int32 Level, TArray<UGraphEdGraphNode*>& Contour);
	
	void ShiftSubTree(UGraphEdGraphNode *RootNode, const FVector2D &Offset);

	void UpdateParentNodePosition(UGraphEdGraphNode *RootNode);
};
