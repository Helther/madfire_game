#pragma once


#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "K2Node.h"
#include "EdGraphSchema_K2.h"
#include "CondGraphSchema.generated.h"

class UFSMEdGraphEdge;

UCLASS(MinimalAPI)
class UFSMCondGraphSchema : public UEdGraphSchema_K2
{
	GENERATED_BODY()

public:
	void CreateDefaultNodesForGraph(UEdGraph &Graph) const override;
	bool CanDuplicateGraph(UEdGraph *InSourceGraph) const override;
	void GetGraphDisplayInformation(const UEdGraph &Graph, FGraphDisplayInfo &DisplayInfo) const override;
	bool ShouldAlwaysPurgeOnModification() const override;
	void HandleGraphBeingDeleted(UEdGraph &GraphBeingRemoved) const override;
	void GetGraphContextActions(FGraphContextMenuBuilder &ContextMenuBuilder) const override;

	bool DoesSupportCollapsedNodes() const override;
	bool DoesSupportEventDispatcher() const override;
		


private:
	static UFSMEdGraphEdge *GetEdgeFromGraph(const UEdGraph *Graph);
};