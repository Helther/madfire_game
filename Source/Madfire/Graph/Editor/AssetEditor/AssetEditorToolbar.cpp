#include "AssetEditorToolbar.h"
#include "AssetEditor.h"
#include "GraphAssetEditorCommands.h"
#include "GraphEditorStyle.h"

#define LOCTEXT_NAMESPACE "GraphAssetEditorToolbar"

void FGraphAssetEditroToolbar::AddGenericGraphToolbar(TSharedPtr<FExtender> Extender)
{
    check(GraphEditor.IsValid());
    TSharedPtr<FGraphAssetEditor> GenericGraphEditorPtr = GraphEditor.Pin();

    TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    ToolbarExtender->AddToolBarExtension(
      "Asset",
      EExtensionHook::After,
      GenericGraphEditorPtr->GetToolkitCommands(),
      FToolBarExtensionDelegate::CreateSP(this, &FGraphAssetEditroToolbar::FillGraphToolbar));
    GenericGraphEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FGraphAssetEditroToolbar::FillGraphToolbar(FToolBarBuilder &ToolbarBuilder)
{
    check(GraphEditor.IsValid());
    TSharedPtr<FGraphAssetEditor> GenericGraphEditorPtr = GraphEditor.Pin();

    ToolbarBuilder.BeginSection("Graph");
    {
        ToolbarBuilder.AddToolBarButton(
          FGraphAssetEditorCommands::Get().GraphSettings,
          NAME_None,
          LOCTEXT("GraphSettings_Label", "Graph Settings"),
          LOCTEXT("GraphSettings_ToolTip", "Show the Graph Settings"),
          FSlateIcon(FAppStyle::GetAppStyleSetName(),"LevelEditor.GameSettings"));
    }
    ToolbarBuilder.EndSection();

    ToolbarBuilder.BeginSection("Util");
    {
        ToolbarBuilder.AddToolBarButton(
          FGraphAssetEditorCommands::Get().AutoArrange,
          NAME_None,
          LOCTEXT("AutoArrange_Label", "Auto Arrange"),
          LOCTEXT("AutoArrange_ToolTip",
                  "Auto arrange nodes, not perfect, but still handy"),
          FSlateIcon(FGraphEditorStyle::GetStyleSetName(), "GenericGraphEditor.AutoArrange"));
    }
    ToolbarBuilder.EndSection();
}


#undef LOCTEXT_NAMESPACE