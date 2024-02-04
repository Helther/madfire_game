#include "FSMEdGraphNode.h"
#include "FSMGraph.h"
#include "../SlateGraph/SFSMGraphNode.h"
#include "FSMEdGraphEdge.h"
#include "IStructureDetailsView.h"
#include "FSMEdGraph.h"


TSharedPtr<SGraphNode> UFSMEdGraphNode::CreateVisualWidget()
{
    SlateNode = SNew(SFSMGraphNode, this);
    return SlateNode;
}

void UFSMEdGraphNode::AllocateDefaultPins()
{
    UEdGraphPin *Inputs = CreatePin(EGPD_Input, TEXT("Transition"), TEXT("In"));
    UEdGraphPin *Outputs = CreatePin(EGPD_Output, TEXT("Transition"), TEXT("Out"));
}

void UFSMEdGraphNode::AutowireNewNode(UEdGraphPin *FromPin)
{
    Super::AutowireNewNode(FromPin);

    if(FromPin)
        if(GetSchema()->TryCreateConnection(FromPin,GetInputPin()))
            FromPin->GetOwningNode()->NodeConnectionListChanged();
}

FText UFSMEdGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromName(AssetNode);
}

void UFSMEdGraphNode::PrepareForCopying()
{
    //TODO: Prepare node
    UEdGraphNode::PrepareForCopying();    
}

void UFSMEdGraphNode::DestroyNode()
{
    auto graph = Cast<UFSMEdGraph>(GetGraph());
    if(!graph) return;
    auto asset = graph->GetGraphAsset();
    if(!asset) return;
    asset->RemoveState(AssetNode);

    UEdGraphNode::DestroyNode();
}

void UFSMEdGraphNode::SetAssetNode(FName index)
{
    AssetNode = index;
}

FName UFSMEdGraphNode::GetAssetNode() const
{
    return AssetNode;
}

bool UFSMEdGraphNode::RenameUniqueNode(const FName &NewName)
{
    auto graph = Cast<UFSMEdGraph>(GetGraph());
    if(!graph) return false;
    auto asset = graph->GetGraphAsset();
    if(!asset) return false;
    
    bool bRenameUnique = asset->Validate(NewName);

    if(bRenameUnique)
    {
        SetAssetNode(NewName);
    }

    return bRenameUnique;
}

void UFSMEdGraphNode::GetTransitionList(TArray<UFSMEdGraphEdge *> &OutTransitions, bool bWantSorted) const
{
    //Normal Transitions
    for(int32 LinkIndex = 0; LinkIndex < Pins[1]->LinkedTo.Num(); ++LinkIndex)
    {
        UEdGraphNode *TargetNode = Pins[1]->LinkedTo[LinkIndex]->GetOwningNode();
        if(UFSMEdGraphEdge *Transition = Cast<UFSMEdGraphEdge>(TargetNode))
        {
            OutTransitions.Add(Transition);
        }
    }
}


UEdGraphPin *UFSMEdGraphNode::GetOutputPin() const { return Pins[1]; }

UEdGraphPin *UFSMEdGraphNode::GetInputPin() const { return Pins[0]; }
