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
