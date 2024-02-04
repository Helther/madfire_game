#include "FSMEdGraphRoot.h"
#include "../SlateGraph/SFSMGraphRoot.h"

UFSMEdGraphRoot::UFSMEdGraphRoot() :
    Super()
{
    AssetNode = "Root"; 
}

UFSMEdGraphRoot::~UFSMEdGraphRoot() 
{
}

TSharedPtr<SGraphNode> UFSMEdGraphRoot::CreateVisualWidget()
{
    SlateNode = SNew(SFSMGraphRoot, this);
    return SlateNode;
}

void UFSMEdGraphRoot::AllocateDefaultPins()
{
    UEdGraphPin *Output = CreatePin(EGPD_Output, TEXT("exec"), TEXT("Root>"));
}

FText UFSMEdGraphRoot::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText();
}

void UFSMEdGraphRoot::PrepareForCopying() {}

void UFSMEdGraphRoot::DestroyNode() {}

UEdGraphNode *UFSMEdGraphRoot::GetOutputNode() const  
{
    if(Pins.Num() > 0 && Pins[0] != nullptr)
    {
        check(Pins[0]->LinkedTo.Num() <= 1);
        if(Pins[0]->LinkedTo.Num() > 0 && Pins[0]->LinkedTo[0]->GetOwningNode() != nullptr)
            return Pins[0]->LinkedTo[0]->GetOwningNode();
    }
    return nullptr;
}