#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "FSMEdGraphSchema.generated.h"


UCLASS()
class UFSMEdGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	void GetGraphContextActions(FGraphContextMenuBuilder &ContextMenuBuilder) const override;

	void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;

	const FPinConnectionResponse CanCreateConnection(const UEdGraphPin *A, const UEdGraphPin *B) const override;

	class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;

	void CreateDefaultNodesForGraph(UEdGraph &Graph) const override;
};