#pragma once 

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "SubclassOf.h"
#include "FSMEdGraphSchemaActions_NewNode.generated.h"


USTRUCT()
struct FSMEdGraphSchemaAction_NewNode : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	FSMEdGraphSchemaAction_NewNode();
	FSMEdGraphSchemaAction_NewNode(FText, FText, FText, const int32);

	UEdGraphNode *PerformAction(class UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectedNewNode = true) override;

	UEdGraphNode * CreateEditorNode(UEdGraph *ParentGraph, bool bSelectNewNode, FName NodeName);

};