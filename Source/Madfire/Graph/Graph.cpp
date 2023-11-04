#include "Graph.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogGraph);

UGraph::UGraph()
{
	NodeType = UGraphNode::StaticClass();
    EdgeType = UGraphEdge::StaticClass();

	bEdgeEnabled = true;

#if WITH_EDITORONLY_DATA
    EdGraph = nullptr;

	bCanRenameNode = true;
#endif
}

UGraph::~UGraph() {}

void UGraph::Print(bool ToConsole, bool ToScreen) 
{
	int Level = 0;
    TArray<UGraphNode *> CurrLevelNodes = RootNodes;
    TArray<UGraphNode *> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
            UGraphNode *Node = CurrLevelNodes[i];
            check(Node != nullptr);

			FString Message = FString::Printf(TEXT("%s, Level %d"), *Node->GetDescription().ToString(), Level);

			if (ToConsole)
			{
                UE_LOG(LogGraph, Log, TEXT("%s"), *Message);
			}

			if (ToScreen && GEngine != nullptr)
                GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, Message);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
                NextLevelNodes.Add(Node->ChildrenNodes[j]);
		}
        CurrLevelNodes = NextLevelNodes;
        NextLevelNodes.Reset();
        ++Level;
	}
}

int UGraph::GetLevelNum() const 
{
    int Level = 0;

	TArray<UGraphNode *> CurrLevelNodes = RootNodes;
    TArray<UGraphNode *> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
            UGraphNode *Node = CurrLevelNodes[i];
            check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
                NextLevelNodes.Add(Node->ChildrenNodes[j]);
		}

		CurrLevelNodes = NextLevelNodes;
        NextLevelNodes.Reset();
        ++Level;
	}

	return Level; 
}

void UGraph::GetNodesByLevel(int Level, TArray<UGraphNode *> &Nodes) 
{
    int CurrLevel = 0;

	TArray<UGraphNode *> NextLevelNodes;
    Nodes = RootNodes;

	while (Nodes.Num() != 0)
	{
        if (CurrLevel == Level)
            break;

		for (int i = 0; i < Nodes.Num(); ++i)
		{
            UGraphNode *Node = Nodes[i];
            check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
                NextLevelNodes.Add(Node->ChildrenNodes[j]);
		}

		Nodes = NextLevelNodes;
        NextLevelNodes.Reset();
        ++CurrLevel;
	}
}

void UGraph::ClearGraph() 
{
	for (int i = 0; i < AllNodes.Num(); ++i)
	{
        UGraphNode *Node = AllNodes[i];

		Node->ParentNodes.Empty();
        Node->ChildrenNodes.Empty();
        Node->Edges.Empty();
	}

	AllNodes.Empty();
    RootNodes.Empty();
}


