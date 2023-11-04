#include "AssetEditor.h"
#include "AssetEditorToolbar.h"
#include "AssetGraphSchema.h"
#include "GraphAssetEditorCommands.h"
#include "GraphEdGraph.h"
#include "AssetToolsModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "GenericCommands.h"
#include "GraphEditorActions.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "Editor/UnrealEd/Public/Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EdGraphUtilities.h"
#include "GraphEdGraph.h"
#include "GraphEdGraphNode.h"
#include "GraphEdGraphEdge.h"
#include "../AutoLayout/TreeLayoutStrategy.h"
#include "../AutoLayout/ForceDirectedLayoutStrategy.h"

#define LOCTEXT_NAMESPACE "AssetEditor_Graph"

const FName GraphEditorAppName = FName(TEXT("GraphEditorApp"));

struct FGraphAssetEditorTabs
{
	// Tab identifiers
	static const FName GenericGraphPropertyID;
	static const FName ViewportID;
	static const FName GenericGraphEditorSettingsID;
};

//////////////////////////////////////////////////////////////////////////

const FName FGraphAssetEditorTabs::GenericGraphPropertyID(TEXT("GenericGraphProperty"));
const FName FGraphAssetEditorTabs::ViewportID(TEXT("Viewport"));
const FName FGraphAssetEditorTabs::GenericGraphEditorSettingsID(TEXT("GenericGraphEditorSettings"));

//////////////////////////////////////////////////////////////////////////

FGraphAssetEditor::FGraphAssetEditor()
{
	EditingGraph = nullptr;

	GenricGraphEditorSettings = NewObject<UGraphAssetEditorSettings>(UGraphAssetEditorSettings::StaticClass());

	OnPackageSavedDelegateHandle = UPackage::PackageSavedEvent.AddRaw(this, &FGraphAssetEditor::OnPackageSaved);
}

FGraphAssetEditor::~FGraphAssetEditor()
{
	UPackage::PackageSavedEvent.Remove(OnPackageSavedDelegateHandle);
}

void FGraphAssetEditor::InitGenericGraphAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UGraph* Graph)
{
	EditingGraph = Graph;
	CreateEdGraph();

	FGenericCommands::Register();
	FGraphEditorCommands::Register();
    FGraphEditorCommands::Register();

	if (!ToolbarBuilder.IsValid())
	{
        ToolbarBuilder = MakeShareable(new FGraphAssetEditroToolbar(SharedThis(this)));
	}

	BindCommands();

	CreateInternalWidgets();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarBuilder->AddGenericGraphToolbar(ToolbarExtender);

	// Layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_GenericGraphEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)->SetHideTabWell(true)
			)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.65f)
					->AddTab(FGraphAssetEditorTabs::ViewportID, ETabState::OpenedTab)->SetHideTabWell(true)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.7f)
						->AddTab(FGraphAssetEditorTabs::GenericGraphPropertyID, ETabState::OpenedTab)->SetHideTabWell(true)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.3f)
						->AddTab(FGraphAssetEditorTabs::GenericGraphEditorSettingsID, ETabState::OpenedTab)
					)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, GraphEditorAppName, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, EditingGraph, false);

	RegenerateMenusAndToolbars();
}

FString FGraphAssetEditor::GetReferencerName() const 
{
	return "Name";
}

void FGraphAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager> &InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_GenericGraphEditor", "Generic Graph Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FGraphAssetEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FGraphAssetEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(FGraphAssetEditorTabs::GenericGraphPropertyID, FOnSpawnTab::CreateSP(this, &FGraphAssetEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Property"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FGraphAssetEditorTabs::GenericGraphEditorSettingsID, FOnSpawnTab::CreateSP(this, &FGraphAssetEditor::SpawnTab_EditorSettings))
		.SetDisplayName(LOCTEXT("EditorSettingsTab", "Generic Graph Editor Setttings"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FGraphAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager> &InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FGraphAssetEditorTabs::ViewportID);
    InTabManager->UnregisterTabSpawner(FGraphAssetEditorTabs::GenericGraphPropertyID);
	InTabManager->UnregisterTabSpawner(FGraphAssetEditorTabs::GenericGraphEditorSettingsID);
}

FName FGraphAssetEditor::GetToolkitFName() const
{
	return FName("FGenericGraphEditor");
}

FText FGraphAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("GenericGraphEditorAppLabel", "Generic Graph Editor");
}

FText FGraphAssetEditor::GetToolkitName() const
{
	const bool bDirtyState = EditingGraph->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("GenericGraphName"), FText::FromString(EditingGraph->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("GenericGraphEditorToolkitName", "{GenericGraphName}{DirtyState}"), Args);
}

FText FGraphAssetEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(EditingGraph);
}

FLinearColor FGraphAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FGraphAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("GenericGraphEditor");
}

FString FGraphAssetEditor::GetDocumentationLink() const
{
	return TEXT("");
}

void FGraphAssetEditor::SaveAsset_Execute()
{
	if (EditingGraph != nullptr)
	{
		RebuildGenericGraph();
	}

	FAssetEditorToolkit::SaveAsset_Execute();
}

void FGraphAssetEditor::AddReferencedObjects(FReferenceCollector &Collector)
{
	Collector.AddReferencedObject(EditingGraph);
	Collector.AddReferencedObject(EditingGraph->EdGraph);
}

UGraphAssetEditorSettings *FGraphAssetEditor::GetSettings() const
{
	return GenricGraphEditorSettings;
}

TSharedRef<SDockTab> FGraphAssetEditor::SpawnTab_Viewport(const FSpawnTabArgs &Args)
{
	check(Args.GetTabId() == FGraphAssetEditorTabs::ViewportID);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"));

	if (ViewportWidget.IsValid())
	{
		SpawnedTab->SetContent(ViewportWidget.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> FGraphAssetEditor::SpawnTab_Details(const FSpawnTabArgs &Args)
{
	check(Args.GetTabId() == FGraphAssetEditorTabs::GenericGraphPropertyID);

	return SNew(SDockTab)
      .Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("Details_Title", "Property"))
		[
			PropertyWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FGraphAssetEditor::SpawnTab_EditorSettings(const FSpawnTabArgs &Args)
{
	check(Args.GetTabId() == FGraphAssetEditorTabs::GenericGraphEditorSettingsID);

	return SNew(SDockTab)
      .Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("EditorSettings_Title", "Generic Graph Editor Setttings"))
		[
			EditorSettingsWidget.ToSharedRef()
		];
}

void FGraphAssetEditor::CreateInternalWidgets()
{
	ViewportWidget = CreateViewportWidget();

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyWidget = PropertyModule.CreateDetailView(Args);
	PropertyWidget->SetObject(EditingGraph);
	PropertyWidget->OnFinishedChangingProperties().AddSP(this, &FGraphAssetEditor::OnFinishedChangingProperties);

	EditorSettingsWidget = PropertyModule.CreateDetailView(Args);
	EditorSettingsWidget->SetObject(GenricGraphEditorSettings);
}

TSharedRef<SGraphEditor> FGraphAssetEditor::CreateViewportWidget()
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_GenericGraph", "Generic Graph");

	CreateCommandList();

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FGraphAssetEditor::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FGraphAssetEditor::OnNodeDoubleClicked);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(EditingGraph->EdGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);
}

void FGraphAssetEditor::BindCommands()
{
	ToolkitCommands->MapAction(FGraphAssetEditorCommands::Get().GraphSettings,
		FExecuteAction::CreateSP(this, &FGraphAssetEditor::GraphSettings),
		FCanExecuteAction::CreateSP(this, &FGraphAssetEditor::CanGraphSettings)
	);

	ToolkitCommands->MapAction(FGraphAssetEditorCommands::Get().AutoArrange,
		FExecuteAction::CreateSP(this, &FGraphAssetEditor::AutoArrange),
		FCanExecuteAction::CreateSP(this, &FGraphAssetEditor::CanAutoArrange)
	);
}

void FGraphAssetEditor::CreateEdGraph()
{
	if (EditingGraph->EdGraph == nullptr)
	{
		EditingGraph->EdGraph = CastChecked<UGraphEdGraph>(FBlueprintEditorUtils::CreateNewGraph(EditingGraph, NAME_None, UGraphEdGraph::StaticClass(), UAssetGraphSchema::StaticClass()));
		EditingGraph->EdGraph->bAllowDeletion = false;

		// Give the schema a chance to fill out any required nodes (like the results node)
		const UEdGraphSchema* Schema = EditingGraph->EdGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*EditingGraph->EdGraph);
	}
}

void FGraphAssetEditor::CreateCommandList()
{
	if (GraphEditorCommands.IsValid())
	{
		return;
	}

	GraphEditorCommands = MakeShareable(new FUICommandList);

	// Can't use CreateSP here because derived editor are already implementing TSharedFromThis<FAssetEditorToolkit>
	// however it should be safe, since commands are being used only within this editor
	// if it ever crashes, this function will have to go away and be reimplemented in each derived class

	GraphEditorCommands->MapAction(FGraphAssetEditorCommands::Get().GraphSettings,
      FExecuteAction::CreateRaw(this, &FGraphAssetEditor::GraphSettings),
      FCanExecuteAction::CreateRaw(this, &FGraphAssetEditor::CanGraphSettings));

	GraphEditorCommands->MapAction(FGraphAssetEditorCommands::Get().AutoArrange,
      FExecuteAction::CreateRaw(this, &FGraphAssetEditor::AutoArrange),
      FCanExecuteAction::CreateRaw(this, &FGraphAssetEditor::CanAutoArrange));

	GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
      FExecuteAction::CreateRaw(this, &FGraphAssetEditor::SelectAllNodes),
      FCanExecuteAction::CreateRaw(this, &FGraphAssetEditor::CanSelectAllNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
      FExecuteAction::CreateRaw(this, &FGraphAssetEditor::DeleteSelectedNodes),
      FCanExecuteAction::CreateRaw(this, &FGraphAssetEditor::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
      FExecuteAction::CreateRaw(this, &FGraphAssetEditor::CopySelectedNodes),
      FCanExecuteAction::CreateRaw(this, &FGraphAssetEditor::CanCopyNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
      FExecuteAction::CreateRaw(this, &FGraphAssetEditor::CutSelectedNodes),
      FCanExecuteAction::CreateRaw(this, &FGraphAssetEditor::CanCutNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
      FExecuteAction::CreateRaw(this, &FGraphAssetEditor::PasteNodes),
      FCanExecuteAction::CreateRaw(this, &FGraphAssetEditor::CanPasteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
      FExecuteAction::CreateRaw(this, &FGraphAssetEditor::DuplicateNodes),
      FCanExecuteAction::CreateRaw(this, &FGraphAssetEditor::CanDuplicateNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Rename,
      FExecuteAction::CreateSP(this, &FGraphAssetEditor::OnRenameNode),
      FCanExecuteAction::CreateSP(this, &FGraphAssetEditor::CanRenameNodes)
	);
}

TSharedPtr<SGraphEditor> FGraphAssetEditor::GetCurrGraphEditor() const
{
	return ViewportWidget;
}

FGraphPanelSelectionSet FGraphAssetEditor::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	TSharedPtr<SGraphEditor> FocusedGraphEd = GetCurrGraphEditor();
	if (FocusedGraphEd.IsValid())
	{
		CurrentSelection = FocusedGraphEd->GetSelectedNodes();
	}

	return CurrentSelection;
}

void FGraphAssetEditor::RebuildGenericGraph()
{
	if (EditingGraph == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("FGenericGraphAssetEditor::RebuildGenericGraph EditingGraph is nullptr"));
		return;
	}

	UGraphEdGraph* EdGraph = Cast<UGraphEdGraph>(EditingGraph->EdGraph);
	check(EdGraph != nullptr);

	EdGraph->RebuildGraph();
}

void FGraphAssetEditor::SelectAllNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		CurrentGraphEditor->SelectAllNodes();
	}
}

bool FGraphAssetEditor::CanSelectAllNodes()
{
	return true;
}

void FGraphAssetEditor::DeleteSelectedNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());

	CurrentGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode* EdNode = Cast<UEdGraphNode>(*NodeIt);
		if (EdNode == nullptr || !EdNode->CanUserDeleteNode())
			continue;;

		if (UGraphEdGraphNode *EdNode_Node = Cast<UGraphEdGraphNode>(EdNode))
		{
			EdNode_Node->Modify();

			const UEdGraphSchema* Schema = EdNode_Node->GetSchema();
			if (Schema != nullptr)
			{
				Schema->BreakNodeLinks(*EdNode_Node);
			}

			EdNode_Node->DestroyNode();
		}
		else
		{
			EdNode->Modify();
			EdNode->DestroyNode();
		}
	}
}

bool FGraphAssetEditor::CanDeleteNodes()
{
	// If any of the nodes can be deleted then we should allow deleting
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node != nullptr && Node->CanUserDeleteNode())
		{
			return true;
		}
	}

	return false;
}

void FGraphAssetEditor::DeleteSelectedDuplicatableNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FGraphPanelSelectionSet OldSelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}

	// Delete the duplicatable nodes
	DeleteSelectedNodes();

	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}
}

void FGraphAssetEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
}

bool FGraphAssetEditor::CanCutNodes()
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FGraphAssetEditor::CopySelectedNodes()
{
	// Export the selected nodes and place the text on the clipboard
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	FString ExportedText;

	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node == nullptr)
		{
			SelectedIter.RemoveCurrent();
			continue;
		}

		if (UGraphEdGraphEdge *EdNode_Edge = Cast<UGraphEdGraphEdge>(*SelectedIter))
		{
            UGraphEdGraphNode *StartNode = EdNode_Edge->GetStartNode();
            UGraphEdGraphNode *EndNode   = EdNode_Edge->GetEndNode();

			if (!SelectedNodes.Contains(StartNode) || !SelectedNodes.Contains(EndNode))
			{
				SelectedIter.RemoveCurrent();
				continue;
			}
		}

		Node->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}

bool FGraphAssetEditor::CanCopyNodes()
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}

void FGraphAssetEditor::PasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		PasteNodesHere(CurrentGraphEditor->GetPasteLocation());
	}
}

void FGraphAssetEditor::PasteNodesHere(const FVector2D &Location)
{
	// Find the graph editor with focus
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}
	// Select the newly pasted stuff
	UEdGraph* EdGraph = CurrentGraphEditor->GetCurrentGraph();

	{
		const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
		EdGraph->Modify();

		// Clear the selection set (newly pasted stuff will be selected)
		CurrentGraphEditor->ClearSelectionSet();

		// Grab the text to paste from the clipboard.
		FString TextToImport;
		FPlatformApplicationMisc::ClipboardPaste(TextToImport);

		// Import the nodes
		TSet<UEdGraphNode*> PastedNodes;
		FEdGraphUtilities::ImportNodesFromText(EdGraph, TextToImport, PastedNodes);

		//Average position of nodes so we can move them while still maintaining relative distances to each other
		FVector2D AvgNodePosition(0.0f, 0.0f);

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* Node = *It;
			AvgNodePosition.X += Node->NodePosX;
			AvgNodePosition.Y += Node->NodePosY;
		}

		float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* Node = *It;
			CurrentGraphEditor->SetNodeSelection(Node, true);

			Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
			Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

			Node->SnapToGrid(16);

			// Give new node a different Guid from the old one
			Node->CreateNewGuid();
		}
	}

	// Update UI
	CurrentGraphEditor->NotifyGraphChanged();

	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}
}

bool FGraphAssetEditor::CanPasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return false;
	}

	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(CurrentGraphEditor->GetCurrentGraph(), ClipboardContent);
}

void FGraphAssetEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

bool FGraphAssetEditor::CanDuplicateNodes()
{
	return CanCopyNodes();
}

void FGraphAssetEditor::GraphSettings()
{
	PropertyWidget->SetObject(EditingGraph);
}

bool FGraphAssetEditor::CanGraphSettings() const
{
	return true;
}

void FGraphAssetEditor::AutoArrange()
{
	UGraphEdGraph* EdGraph = Cast<UGraphEdGraph>(EditingGraph->EdGraph);
	check(EdGraph != nullptr);

	const FScopedTransaction Transaction(LOCTEXT("GenericGraphEditorAutoArrange", "Generic Graph Editor: Auto Arrange"));

	EdGraph->Modify();

	UAutoLayoutStrategy* LayoutStrategy = nullptr;
	switch (GenricGraphEditorSettings->AutoLayoutStrategy)
	{
	case EAutoLayoutStrategy::Tree:
		LayoutStrategy = NewObject<UAutoLayoutStrategy>(EdGraph, UTreeLayoutStrategy::StaticClass());
		break;
	case EAutoLayoutStrategy::ForceDirected:
		LayoutStrategy = NewObject<UAutoLayoutStrategy>(EdGraph, UForceDirectedLayoutStrategy::StaticClass());
		break;
	default:
		break;
	}

	if (LayoutStrategy != nullptr)
	{
		LayoutStrategy->Settings = GenricGraphEditorSettings;
		LayoutStrategy->Layout(EdGraph);
		LayoutStrategy->ConditionalBeginDestroy();
	}
	else
	{
        UE_LOG(LogGraph, Log, TEXT("FAssetEditor_GenericGraph::AutoArrange LayoutStrategy is null."));
	}
}

bool FGraphAssetEditor::CanAutoArrange() const
{
	return EditingGraph != nullptr && Cast<UGraphEdGraph>(EditingGraph->EdGraph) != nullptr;
}

void FGraphAssetEditor::OnRenameNode()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UEdGraphNode* SelectedNode = Cast<UEdGraphNode>(*NodeIt);
			if (SelectedNode != NULL && SelectedNode->bCanRenameNode)
			{
				CurrentGraphEditor->IsNodeTitleVisible(SelectedNode, true);
				break;
			}
		}
	}
}

bool FGraphAssetEditor::CanRenameNodes() const
{
    UGraphEdGraph *EdGraph = Cast<UGraphEdGraph>(EditingGraph->EdGraph);
	check(EdGraph != nullptr);

	UGraph* Graph = EdGraph->GetGraph();
	check(Graph != nullptr)

	return Graph->bCanRenameNode && GetSelectedNodes().Num() == 1;
}

void FGraphAssetEditor::OnSelectedNodesChanged(const TSet<class UObject *> &NewSelection)
{
	TArray<UObject*> Selection;

	for (UObject* SelectionEntry : NewSelection)
	{
		Selection.Add(SelectionEntry);
	}

	if (Selection.Num() == 0) 
	{
		PropertyWidget->SetObject(EditingGraph);

	}
	else
	{
		PropertyWidget->SetObjects(Selection);
	}
}

void FGraphAssetEditor::OnNodeDoubleClicked(UEdGraphNode *Node) {
	
}

void FGraphAssetEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (EditingGraph == nullptr)
		return;

	EditingGraph->EdGraph->GetSchema()->ForceVisualizationCacheClear();
}

void FGraphAssetEditor::OnPackageSaved(const FString &PackageFileName, UObject *Outer)
{
	RebuildGenericGraph();
}

void FGraphAssetEditor::RegisterToolbarTab(const TSharedRef<class FTabManager>& InTabManager) 
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}


#undef LOCTEXT_NAMESPACE

