#include "CondGraphResult.h"

void UFSMConditionResultNode::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{}

void UFSMConditionResultNode::PreEditChange(FProperty *PropertyAboutToChanged) 
{
}

void UFSMConditionResultNode::PostEditUndo() 
{
}

void UFSMConditionResultNode::Serialize(FArchive &Ar) 
{
}

void UFSMConditionResultNode::AllocateDefaultPins() 
{
    

}

FLinearColor UFSMConditionResultNode::GetNodeTitleColor() const 
{
    return FLinearColor::Red; 
}
/*
FText UFSMConditionResultNode::GetTooltipText() const 
{
    return FText::FromString("Condition to transition Result"); 
}

FText UFSMConditionResultNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromString("Result");
}

bool UFSMConditionResultNode::IsNodeRootSet() const 
{
    return false;
}

bool UFSMConditionResultNode::ShowVisualWarning() const 
{
    return false; 
}

FText UFSMConditionResultNode::GetVisualWarningTooltipText() const 
{
    return FText();
}
*/
void UFSMConditionResultNode::GetPinHoverText(const UEdGraphPin &Pin, FString &HoverTextOut) const
{}

bool UFSMConditionResultNode::ShowPaletteIconOnNode() const 
{
    return false; 
}

void UFSMConditionResultNode::PinDefaultValueChanged(UEdGraphPin *Pin) 
{
}

FString UFSMConditionResultNode::GetPinMetaData(FName InPinName, FName InKey)
{
    return FString();
}

void UFSMConditionResultNode::AddSearchMetaDataInfo(TArray<struct FSearchTagDataPair> &OutTaggedMetaData) const
{}

void UFSMConditionResultNode::AddPinSearchMetaDataInfo(const UEdGraphPin *Pin, TArray<struct FSearchTagDataPair> &OutTaggedMetaData) const
{}

void UFSMConditionResultNode::PinConnectionListChanged(UEdGraphPin *Pin) 
{
}

void UFSMConditionResultNode::AutowireNewNode(UEdGraphPin *FropPin) 
{
}

void UFSMConditionResultNode::PostPlacedNewNode() 
{
}

void UFSMConditionResultNode::PostPasteNode() 
{
}

void UFSMConditionResultNode::DestroyNode() 
{
}
/*
bool UFSMConditionResultNode::CanUserDeleteNode() const 
{
    return false; 
}

bool UFSMConditionResultNode::CanDuplicateNode() const 
{
    return false; 
}*/

void UFSMConditionResultNode::ReconstructNode() 
{
}

bool UFSMConditionResultNode::NodeCausesStructuralBlueprintChange() const
{
    return false;
}

bool UFSMConditionResultNode::ShouldShowNodeProperties() const 
{
    return false; 
}

bool UFSMConditionResultNode::CanPlaceBreakpoints() const 
{
    return false; 
}

void UFSMConditionResultNode::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin *> &OldPins)
{
}

bool UFSMConditionResultNode::CanCreateUnderSpecifiedSchema(const UEdGraphSchema *DesiredSchema) const
{
    return false;
}

void UFSMConditionResultNode::GetNodeAttributes(TArray<TKeyValuePair<FString, FString>> &OutNodeAttributes) const
{}

void UFSMConditionResultNode::GetMenuActions(FBlueprintActionDatabaseRegistrar &ActionRegistrar) const
{}

FText UFSMConditionResultNode::GetMenuCategory() const 
{
    return FText(); 
}

void UFSMConditionResultNode::ExpandNode(FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph)
{}

UObject *UFSMConditionResultNode::GetJumpTargetForDoubleClick() const
{
    return nullptr; 
}

bool UFSMConditionResultNode::CanJumpToDefinition() const 
{
    return false; 
}

void UFSMConditionResultNode::JumpToDefinition() const 
{
}

void UFSMConditionResultNode::HandleVariableRenamed(UBlueprint *InBlueprint, UClass *InVariableClass,UEdGraph *InGraph,const FName &InOldVarName, const FName &InNewVarName)
{}

void UFSMConditionResultNode::ReplaceReferences(UBlueprint *InBLueprint, UBlueprint *InReplacementBlueprint,const FMemberReference &InSource, const FMemberReference &InReplacement)
{}

bool UFSMConditionResultNode::ReferencesVariable(const FName &InVarName, const UStruct *InScope) const
{
    return false;
}

bool UFSMConditionResultNode::ReferencesFunction(const FName &InFunctionName, const UStruct *InScope) const
{
    return false;
}

