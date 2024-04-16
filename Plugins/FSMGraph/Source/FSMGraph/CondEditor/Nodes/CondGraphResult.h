#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "Editor.h"
#include "IPropertyAccessEditor.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UnrealType.h"
#include "CondGraphResult.generated.h"

UCLASS(MinimalAPI)
class UFSMConditionResultNode : public UK2Node
{
	GENERATED_BODY()

public:
	// UObject interface
	void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
	void PreEditChange(FProperty *PropertyAboutToChanged) override;
	void PostEditUndo() override;
	void Serialize(FArchive &Ar) override;

	//UEdGraphNode interface
	void AllocateDefaultPins() override;
	FLinearColor GetNodeTitleColor() const override;
	void GetPinHoverText(const UEdGraphPin &Pin, FString &HoverTextOut) const override;
	bool ShowPaletteIconOnNode() const override;
	void PinDefaultValueChanged(UEdGraphPin *Pin) override;
	FString GetPinMetaData(FName InPinName, FName InKey);
	void AddSearchMetaDataInfo(TArray<struct FSearchTagDataPair> &OutTaggedMetaData) const override;
	void AddPinSearchMetaDataInfo(const UEdGraphPin *Pin, TArray<struct FSearchTagDataPair> &OutTaggedMetaData) const override;
	void PinConnectionListChanged(UEdGraphPin *Pin) override;
	void AutowireNewNode(UEdGraphPin *FropPin) override;
	void PostPlacedNewNode() override;
	void PostPasteNode() override;
	void DestroyNode() override;

	//UK2Node interface
	void ReconstructNode() override;
	bool NodeCausesStructuralBlueprintChange() const override;
	bool ShouldShowNodeProperties() const override;
	bool CanPlaceBreakpoints() const override;
	void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*> &OldPins) override;
	bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema *DesiredSchema) const override;
	void GetNodeAttributes(TArray<TKeyValuePair<FString, FString>> &OutNodeAttributes) const override;
	void GetMenuActions(FBlueprintActionDatabaseRegistrar &ActionRegistrar) const override;
	FText GetMenuCategory() const override;
	void ExpandNode(class FKismetCompilerContext &CompilerContext, UEdGraph *SourceGraph) override;
	//

	UObject *GetJumpTargetForDoubleClick() const override;
	bool CanJumpToDefinition() const override;
	void JumpToDefinition() const override;
	void HandleVariableRenamed(UBlueprint *InBlueprint, UClass *InVariableClass, UEdGraph *InGraph, const FName &InOldVarName, const FName &InNewVarName) override;
	void ReplaceReferences(UBlueprint *InBLueprint, UBlueprint *InReplacementBlueprint, const FMemberReference &InSource, const FMemberReference &InReplacement) override;
	bool ReferencesVariable(const FName &InVarName, const UStruct *InScope) const override;
	bool ReferencesFunction(const FName &InFunctionName, const UStruct *InScope) const override;

};