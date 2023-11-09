#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "FSMEdGraph.generated.h"


class UFSMGraph;

UCLASS()
class UFSMEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	bool IsNameUnique(const FText &InName);
	void SaveGraph();
	void ClearOldLinks();
	void LinkAssetNodes();
	void RefreshNodes();
	void MapNamedNodes();
	UFSMGraph *GetGraphAsset();

};