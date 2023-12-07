#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "SubclassOf.h"
#include "FSMEdGraphSchemaActions_NewEdge.generated.h"

USTRUCT()
struct FSMEdGraphSchemaAction_NewEdge : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	FSMEdGraphSchemaAction_NewEdge();
	FSMEdGraphSchemaAction_NewEdge(FText, FText, FText, const int32);

	UEdGraphNode *PerformAction(class UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectedNewNode = true) override;
	void AddReferencedObjects(FReferenceCollector &Collector) override;
	UEdGraphNode * CreateEditorNode(UEdGraph *ParentGraph, bool bSelectNewNode, FName NodeName);


};

