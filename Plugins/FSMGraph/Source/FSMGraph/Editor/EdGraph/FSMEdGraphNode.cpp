#include "FSMEdGraphNode.h"
#include "FSMGraph.h"
#include "../SlateGraph/SFSMGraphNode.h"
#include "FSMEdGraphEdge.h"
#include "IStructureDetailsView.h"
#include "FSMEdGraph.h"


UFSMEdGraphNode::UFSMEdGraphNode() 
{
    EdNodeName = FText::FromString("FUCK");
}

UFSMEdGraphNode::~UFSMEdGraphNode() 
{}

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
    return FText::FromString("State Node");
    switch(TitleType)
    {
        case ENodeTitleType::Type::MenuTitle:
            return FText::FromString("State Node");
        default:
            FText Title = GetEdNodeName();
            return Title;
    }
}

void UFSMEdGraphNode::PrepareForCopying()
{
    //TODO: Prepare node
    UEdGraphNode::PrepareForCopying();    
}

void UFSMEdGraphNode::DestroyNode()
{
    UEdGraphNode::DestroyNode();
}

void UFSMEdGraphNode::SetAssetNode(FName index)
{
    AssetNode = RealName = index;
}

FName UFSMEdGraphNode::GetAssetNode() const
{
    return RealName;
}

void UFSMEdGraphNode::PostCopyNode()
{
    
}

bool UFSMEdGraphNode::RenameUniqueNode(const FText &NewName)
{
    bool bRenameNode = false;

    return bRenameNode;
}

FText UFSMEdGraphNode::GetEdNodeName() const
{
    return EdNodeName;
}

void UFSMEdGraphNode::SetEdNodeName(const FText &Name)
{
    EdNodeName = Name;
}

void UFSMEdGraphNode::SetEdNodeName(const FName &Name)
{
    EdNodeName = FText::FromName(Name);
}

TSharedPtr<SWidget> UFSMEdGraphNode::GetContentWidget()
{
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bAllowSearch = false;
    DetailsViewArgs.bLockable = false;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

    /** Camera Feed Info Details View */
	//FStructureDetailsViewArgs StructDetailsView;
    
    FPropertyEditorModule &PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    TSharedPtr<IDetailsView> DefaultView = PropertyEditorModule.CreateDetailView(DetailsViewArgs); 
    DefaultView->SetObject(this);

    return DefaultView;
}

void UFSMEdGraphNode::PostEditChangeProperty(struct FPropertyChangedEvent &e)
{
    Super::PostEditChangeProperty(e);

    auto graph = CastChecked<UFSMEdGraph>(GetGraph())->GetGraphAsset();

    FName PropertyName = (e.Property != nullptr) ? e.Property->GetFName() : NAME_None;
    if(PropertyName == GET_MEMBER_NAME_CHECKED(UFSMEdGraphNode, AssetNode))
    {
        if(!graph->Validate(AssetNode))
            AssetNode = RealName;
        else
        {
            auto state = graph->GetState(RealName);
            graph->RemoveState(RealName);
            RealName = AssetNode;
            graph->AddState(RealName, state);
        }   
        //Check AssetNode is valid
    }
    else if(PropertyName == GET_MEMBER_NAME_CHECKED(UFSMEdGraphNode, IsInit))
    {
        //Check is One is Valid
    }
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

void UFSMEdGraphNode::UpdateVisualNode()
{
    
}

void UFSMEdGraphNode::SaveNodesAsChildren(TArray<UEdGraphNode *> &Children)
{
    
}

UEdGraphPin *UFSMEdGraphNode::GetOutputPin() const { return Pins[0]; }

UEdGraphPin *UFSMEdGraphNode::GetInputPin() const { return Pins[1]; }

bool UFSMEdGraphNode::HasOutputPins()
{
    return true;
}

bool UFSMEdGraphNode::HasInputPins()
{
    return true;
}
