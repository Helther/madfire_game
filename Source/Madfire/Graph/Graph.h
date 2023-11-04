#pragma once


#include "CoreMinimal.h"
#include "GraphNode.h"
#include "GraphEdge.h"
#include "GameplayTagContainer.h"
#include "Graph.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGraph, Log, All);


UCLASS(Blueprintable)
class UGraph : public UObject
{
    GENERATED_BODY()

public:
    UGraph();
    ~UGraph();

    UPROPERTY(EditDefaultsOnly, Category = "Graph")
    TSubclassOf<UGraphNode> NodeType;

    UPROPERTY(EditDefaultsOnly, Category = "Graph")
    TSubclassOf<UGraphEdge> EdgeType;

    //UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Graph")
    //FGameplayTagContainer Tags;

    UPROPERTY(BlueprintReadOnly, Category = "Graph")
    TArray<UGraphNode *> RootNodes;

    UPROPERTY(BlueprintReadOnly, Category = "Graph")
    TArray<UGraphNode *> AllNodes;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Graph")
    bool bEdgeEnabled;

    UFUNCTION(BlueprintCallable, Category = "Graph")
    void Print(bool ToConsole = true, bool ToScreen = true);

    UFUNCTION(BlueprintCallable, Category = "Graph")
    int GetLevelNum() const;

    UFUNCTION(BlueprintCallable, Category = "Graph")
    void GetNodesByLevel(int Level, TArray<UGraphNode *> &Nodes);

    void ClearGraph();


#if WITH_EDITORONLY_DATA
    UPROPERTY()
    class UEdGraph *EdGraph;

    UPROPERTY(EditDefaultsOnly, Category = "Graph")
    bool bCanRenameNode;
#endif

};