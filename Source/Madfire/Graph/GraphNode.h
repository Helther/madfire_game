#pragma once

#include "CoreMinimal.h"
#include "GraphNode.generated.h"


class UGraph;
class UGraphEdge;

UCLASS(Blueprintable)
class UGraphNode : public UObject
{
    GENERATED_BODY()

public:
    UGraphNode();
    ~UGraphNode();

    UPROPERTY(VisibleDefaultsOnly, Category = "Graph")
    UGraph *Graph;

    UPROPERTY(BlueprintReadOnly, Category = "Graph")
    TArray<UGraphNode *> ParentNodes;

    UPROPERTY(BlueprintReadOnly)
    TArray<UGraphNode *> ChildrenNodes;

    UPROPERTY(BlueprintReadOnly)
    TMap<UGraphNode *, UGraphEdge *> Edges;

    UFUNCTION(BlueprintCallable)
    virtual UGraphEdge *GetEdge(UGraphNode *ChildNode);

    UFUNCTION(BlueprintCallable)
    bool IsLeafNode() const;

    UFUNCTION(BlueprintCallable)
    UGraph *GetGraph() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    FText GetDescription() const;

    virtual FText GetDescription_Implementation() const;


#if WITH_EDITORONLY_DATA
    UPROPERTY(EditDefaultsOnly)
    FText NodeTitle;

    UPROPERTY(VisibleDefaultsOnly)
    TSubclassOf<UGraph> CompatibleGraphType;

    UPROPERTY(EditDefaultsOnly)
    FLinearColor BackgroundColor;

    UPROPERTY(EditDefaultsOnly)
    FText ContextMenuName;
#endif

#if WITH_EDITOR
    virtual FLinearColor GetBackgroundColor() const;

    virtual FText GetNodeTitle() const;

    virtual void SetNodeTitle(const FText &NewTitle);

    virtual bool CanCreateConnection(UGraphNode *Other, FText &ErrorMessage);        
#endif
};