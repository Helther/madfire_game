#include "FSMEdGraphNode.h"
#include "FSMGraph.h"
#include "../SlateGraph/SFSMGraphNode.h"
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
    UEdGraphNode::AllocateDefaultPins();
    if(HasInputPins())
        CreatePin(EGPD_Input, FName("InCategory"), FName("InPinName"));
    if(HasOutputPins())
        CreatePin(EGPD_Output, FName("OutCategory"), FName("OutPinName"));
}

FText UFSMEdGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    switch(TitleType)
    {
        case ENodeTitleType::Type::MenuTitle:
            return FText::FromName(AssetNode);
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

void UFSMEdGraphNode::AutowireNewNode(UEdGraphPin *FromPin)
{
    if(FromPin)
    {
        UEdGraphNode::AutowireNewNode(FromPin);
        if(FromPin->Direction == EEdGraphPinDirection::EGPD_Input)
        {
            if(GetSchema()->TryCreateConnection(FromPin, FindPin(FName("OutPinName"))))
            {
                FromPin->GetOwningNode()->NodeConnectionListChanged();
            }        
        }
        if(FromPin->Direction == EEdGraphPinDirection::EGPD_Output)
        {
            if(GetSchema()->TryCreateConnection(FromPin, FindPin(FName("InPinName"))))
            {
                FromPin->GetOwningNode()->NodeConnectionListChanged();            
            }        
        }
    }
}

void UFSMEdGraphNode::SetAssetNode(FName index)
{
    AssetNode = index;
}

FName UFSMEdGraphNode::GetAssetNode() const
{
    return AssetNode;
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
	FStructureDetailsViewArgs StructDetailsView;
    
    FPropertyEditorModule &PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    TSharedPtr<IDetailsView> DefaultView = PropertyEditorModule.CreateDetailView(DetailsViewArgs); 

    UFSMGraph *graph = Cast<UFSMEdGraph>(GetGraph())->GetGraphAsset();
    auto name = AssetNode;
    if(!graph)
        return DefaultView;

    auto &structinfo = graph->StatesMap[name];
    TSharedRef<FStructOnScope> LensInfoStructOnScope = MakeShared<FStructOnScope>(FSMState::StaticStruct(), reinterpret_cast<uint8*>(&structinfo));
    TSharedPtr<IStructureDetailsView> View = PropertyEditorModule.CreateStructureDetailView(DetailsViewArgs, StructDetailsView, LensInfoStructOnScope);
    
    return View->GetWidget();
}

void UFSMEdGraphNode::UpdateVisualNode()
{
    
}

void UFSMEdGraphNode::SaveNodesAsChildren(TArray<UEdGraphNode *> &Children)
{
    
}

bool UFSMEdGraphNode::HasOutputPins()
{
    return true;
}

bool UFSMEdGraphNode::HasInputPins()
{
    return true;
}
