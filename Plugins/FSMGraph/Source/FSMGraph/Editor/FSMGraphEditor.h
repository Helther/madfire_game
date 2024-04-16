#pragma once

#include "CoreMinimal.h"
#include "AssetEditorToolkit.h"
#include "NotifyHook.h"
#include "GraphEditor.h"


class UFSMGraph;

class FSMGraphAssetEditor : public FAssetEditorToolkit, public FNotifyHook
{
public:
	FSMGraphAssetEditor();
	~FSMGraphAssetEditor();

	// FAssetEditorToolkit interface
	FLinearColor GetWorldCentricTabColorScale() const override;
	FName GetToolkitFName() const override;
	FText GetBaseToolkitName() const override;
	FString GetWorldCentricTabPrefix() const override;
	void RegisterTabSpawners(const TSharedRef<FTabManager> &TabManager) override;
	void UnregisterTabSpawners(const TSharedRef<FTabManager> &TabManager) override;
	void SaveAsset_Execute() override;

	void BlueprintCompiled();
	void InitGraphAssetEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost> &InToolkitHost, UFSMGraph *InGraph);

private:
	UFSMGraph *GraphAsset;

	TSharedPtr<SGraphEditor> EdGraphEditor;
	TSharedPtr<IDetailsView> DetailsWidget;
	TSharedPtr<FUICommandList> GraphEditorCommands;

	static const FName DetailsTabId;
	static const FName GraphTabId;

	FGraphPanelSelectionSet GetSelectedNodes();
	void BindToolkitCommands();

	TSharedRef<SDockTab> HandleTabManagerSpawnTabDetails(const FSpawnTabArgs &Args);
	TSharedRef<SDockTab> HandleTabManagerSpawnTabGraph(const FSpawnTabArgs &Args);
	TSharedRef<SDockTab> HandleTabManagerSpawnTabCond(const FSpawnTabArgs &Args);
	//TSharedRef<SDockTab> HandleTabManagerSpawnTabGraphDetails(const FSpawnTabArgs &Args);

	UEdGraph *CurrendCond;

	//Delegates
	bool CanSelectAllNodes();
	bool CanCopyNodes();
	bool CanPasteNodes();
	bool CanCutNodes();
	bool CanDuplicateNodes();
	bool CanDeleteNodes();

	void OnCommandSelectAllNodes();
	void OnCommandCopy();
	void OnCommandPaste();
	void OnCommandCut();
	void OnCommandDuplicated();
	void OnCommandDelete();
};