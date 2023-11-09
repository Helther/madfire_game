#include "FSMGraphEditor.h"
#include "PropertyEditorModule.h"
#include "Slate.h"
#include "Editor.h"
#include "FSMGraph.h"
#include "BlueprintEditorUtils.h"
#include "GraphEditorActions.h"
#include "PlatformApplicationMisc.h"
#include "EdGraphUtilities.h"
#include "ModuleManager.h"
#include "EdGraph/FSMEdGraph.h"
#include "EdGraph/FSMEdGraphNode.h"
#include "EdGraph/FSMEdGraphSchema.h"

const FName FSMGraphAssetEditor::DetailsTabId(TEXT("FSMGraphEditorDetailsTabID"));
const FName FSMGraphAssetEditor::GraphTabId(TEXT("FSMGraphEditorGraphTabID"));

FSMGraphAssetEditor::FSMGraphAssetEditor()
{
	GEditor->OnBlueprintCompiled().AddRaw(this, &FSMGraphAssetEditor::BlueprintCompiled);
}


FSMGraphAssetEditor::~FSMGraphAssetEditor()
{
	GEditor->OnBlueprintCompiled().RemoveAll(this);
}

FName FSMGraphAssetEditor::GetToolkitFName() const
{
	return FName("FSMGraphEditor");
}

FText FSMGraphAssetEditor::GetBaseToolkitName() const
{
	return FText::FromString("FSM Graph Editor");
}

FString FSMGraphAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("FSMGraphEditor");
}

FLinearColor FSMGraphAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

void FSMGraphAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager> &TabManager)
{
	WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(FText::FromString("YetAnotherToolkitWorkspaceMenu"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	TabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FSMGraphAssetEditor::HandleTabManagerSpawnTabDetails))
		.SetDisplayName(FText::FromString("Details Tab"))
		.SetGroup(WorkspaceMenuCategoryRef);

	TabManager->RegisterTabSpawner(GraphTabId, FOnSpawnTab::CreateSP(this, &FSMGraphAssetEditor::HandleTabManagerSpawnTabGraph))
		.SetDisplayName(FText::FromString("Graph Editor"))
		.SetGroup(WorkspaceMenuCategoryRef);

	FAssetEditorToolkit::RegisterTabSpawners(TabManager);

	/*const auto &LocalCategories = TabManager.Get().GetLocalWorkspaceMenuRoot()->GetChildItems();
	TSharedRef<FWorkspaceItem> ToolbarSpawnerCategory = LocalCategories.Num() > 0 ? LocalCategories[0] : TabManager->GetLocalWorkspaceMenuRoot();
	
	TabManager->RegisterTabSpawner(ToolbarTabId, FOnSpawnTab::CreateSP(this, &FAssetEditorToolkit::SpawnTab_Toolbar))
		.SetDisplayName(LOCTEXT("ToolbarTab", "Toolbar"))
		.SetGroup(ToolbarSpawnerCategory)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Toolbar.Icon"));*/
}

void FSMGraphAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager> &TabManager) 
{
	FAssetEditorToolkit::UnregisterTabSpawners(TabManager);
	TabManager->UnregisterTabSpawner(DetailsTabId);
	TabManager->UnregisterTabSpawner(GraphTabId);
}

void FSMGraphAssetEditor::InitGraphAssetEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost> &InToolkitHost, UFSMGraph *InGraph)
{
	GraphAsset = InGraph;

	auto graph = GraphAsset->EdGraph;
	if(!graph)
    {
		//FBlueprintEditorUtils::CreateNewGraph()
		//Add Ed Graph

		GraphAsset->EdGraph = CastChecked<UFSMEdGraph>(FBlueprintEditorUtils::CreateNewGraph(GraphAsset,NAME_None,UFSMEdGraph::StaticClass(), UFSMEdGraphSchema::StaticClass()));
		GraphAsset->EdGraph->bAllowDeletion = false;
	}

	FGenericCommands::Register();
	FGraphEditorCommands::Register();
	BindToolkitCommands();

	TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("LayoutName")
        ->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
				->SetHideTabWell(true)
				->SetSizeCoefficient(0.2f)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.8f)
				->Split
				(
					FTabManager::NewStack()
					->AddTab(DetailsTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.15f)
				)
				->Split
				(
					FTabManager::NewStack()
					->AddTab(GraphTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.85f)
				)
			)
		);

	FAssetEditorToolkit::InitAssetEditor(InMode, InToolkitHost, FName("GraphEditorIdentifier"), Layout, true, true, GraphAsset);
}


void FSMGraphAssetEditor::SaveAsset_Execute() 
{
	UE_LOG(LogTemp, Log, TEXT("SAVE ASSET"));

	FAssetEditorToolkit::SaveAsset_Execute();
}

void FSMGraphAssetEditor::BlueprintCompiled()
{

}

FGraphPanelSelectionSet FSMGraphAssetEditor::GetSelectedNodes()
{
	return EdGraphEditor->GetSelectedNodes();
}

void FSMGraphAssetEditor::BindToolkitCommands()
{
	if(!GraphEditorCommands.IsValid())
    {
		GraphEditorCommands = MakeShareable(new FUICommandList());

		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::OnCommandSelectAllNodes),
			FCanExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::CanSelectAllNodes)
		);

		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().Cut,
			FExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::OnCommandCut),
			FCanExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::CanCutNodes)
		);

    	GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().Copy,
			FExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::OnCommandCopy),
			FCanExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::CanCopyNodes)
		);

		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().Paste,
			FExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::OnCommandPaste),
			FCanExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::CanPasteNodes)
		);
	
		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::OnCommandDuplicated),
			FCanExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::CanDuplicateNodes)
		);

		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().Delete,
			FExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::OnCommandDelete),
			FCanExecuteAction::CreateRaw(this, &FSMGraphAssetEditor::CanDeleteNodes)
		);
	}
}

TSharedRef<SDockTab> FSMGraphAssetEditor::HandleTabManagerSpawnTabDetails(const FSpawnTabArgs &Args)
{
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bCustomNameAreaLocation = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;

	FPropertyEditorModule &PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	DetailsWidget = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsWidget->SetObject(GraphAsset);

	return SNew(SDockTab).TabRole(ETabRole::PanelTab)
		[
			DetailsWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FSMGraphAssetEditor::HandleTabManagerSpawnTabGraph(const FSpawnTabArgs &Args)
{
	if(GraphAsset->EdGraph)
    {
		EdGraphEditor = SNew(SGraphEditor)
			.AdditionalCommands(GraphEditorCommands)
			.GraphToEdit(GraphAsset->EdGraph);
	}
    else
    {
		// TODO: Error No Editor Graph 
    }
	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			EdGraphEditor.ToSharedRef()
		];
}

//Delegates
bool FSMGraphAssetEditor::CanSelectAllNodes()
{
	return true;
}
bool FSMGraphAssetEditor::CanCopyNodes()
{
	return true;
}
bool FSMGraphAssetEditor::CanPasteNodes()
{
	return true;
}
bool FSMGraphAssetEditor::CanCutNodes()
{
	return true;
}
bool FSMGraphAssetEditor::CanDuplicateNodes()
{
	return true;
}
bool FSMGraphAssetEditor::CanDeleteNodes()
{
	return true;
}

void FSMGraphAssetEditor::OnCommandSelectAllNodes()
{
	if(EdGraphEditor.IsValid())
		EdGraphEditor->SelectAllNodes();
}

void FSMGraphAssetEditor::OnCommandCopy()
{
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	FString ExportedText;

	for(FGraphPanelSelectionSet::TIterator It(SelectedNodes); It; ++It)
    {
		UEdGraphNode *Node = Cast<UEdGraphNode>(*It);
		if(Node)
			Node->PrepareForCopying();
        else
			It.RemoveCurrent();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	//FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	for(FGraphPanelSelectionSet::TIterator It(SelectedNodes); It; ++It)
    {
		UFSMEdGraphNode *Node = Cast<UFSMEdGraphNode>(*It);
		if(Node)
			Node->PostCopyNode();
	}
}

void FSMGraphAssetEditor::OnCommandPaste()
{
	const FVector2D PasteLocation = EdGraphEditor->GetPasteLocation();

	UEdGraph *EdGraph = EdGraphEditor->GetCurrentGraph();
	EdGraph->Modify();
	EdGraphEditor->ClearSelectionSet();

	FString ExportedText;
	//FPlatformApplicationMisc::ClipboardPaste(ExportedText);
	TSet<UEdGraphNode*> ImportedNodes;
	FEdGraphUtilities::ImportNodesFromText(EdGraph, ExportedText, ImportedNodes);

	for(TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
    {
		UFSMEdGraphNode *Node = Cast<UFSMEdGraphNode>(*It);
		//GraphAsset->AddNode(Node->AssetNode);
	}

	FVector2D AvgNodePosition(0.f, 0.f);

	for(TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
    {
		UEdGraphNode *Node = *It;
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}

	float InvNumNodes = 1.f / float(ImportedNodes.Num());
	AvgNodePosition.X *= InvNumNodes;
	AvgNodePosition.Y *= InvNumNodes;

	for(TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
    {
		UEdGraphNode *Node = *It;
		EdGraphEditor->SetNodeSelection(Node, true);
		
		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + PasteLocation.X;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + PasteLocation.Y;

		Node->SnapToGrid(16);

		Node->CreateNewGuid();
	}

	EdGraphEditor->NotifyGraphChanged();

	UObject *GraphOwner = EdGraph->GetOuter();
	if(GraphOwner)
    {
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}
}

void FSMGraphAssetEditor::OnCommandCut()
{
	OnCommandCopy();

	const FGraphPanelSelectionSet OldSelectedNodes = EdGraphEditor->GetSelectedNodes();
	EdGraphEditor->ClearSelectionSet();
	
	for(FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
    {
		UEdGraphNode *Node = Cast<UEdGraphNode>(*It);
		if(Node && Node->CanDuplicateNode())
        {
			EdGraphEditor->SetNodeSelection(Node, true);
		}
	}

	OnCommandDelete();

	EdGraphEditor->ClearSelectionSet();

	for(FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
    {
		UEdGraphNode *Node = Cast<UEdGraphNode>(*It);
		if(Node)
			EdGraphEditor->SetNodeSelection(Node, true);
	}
}

void FSMGraphAssetEditor::OnCommandDuplicated()
{
	OnCommandCopy();
	OnCommandPaste();
}
void FSMGraphAssetEditor::OnCommandDelete()
{
	EdGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	EdGraphEditor->ClearSelectionSet();

	for(FGraphPanelSelectionSet::TConstIterator It(SelectedNodes); It; ++It)
    {
		if(UEdGraphNode *Node = Cast<UEdGraphNode>(*It))
        {
			Node->Modify();
			Node->DestroyNode();        
		}
	}
}