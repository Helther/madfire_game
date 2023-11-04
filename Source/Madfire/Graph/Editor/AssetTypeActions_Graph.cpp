#include "AssetTypeActions_Graph.h"
#include "AssetEditor/AssetEditor.h"

#define LOCTEXT_NAMESPACE "GraphAssetTypeActions"

FAssetTypeActions_Graph::FAssetTypeActions_Graph(EAssetTypeCategories::Type InAssetCategory) : 
    MyAssetCategory(InAssetCategory)
{}

FText FAssetTypeActions_Graph::GetName() const
{
    return LOCTEXT("FGenericGraphAssetTypeActionsName", "Graph");
}

FColor FAssetTypeActions_Graph::GetTypeColor() const
{
    return FColor(129, 196, 115);
}

UClass *FAssetTypeActions_Graph::GetSupportedClass() const
{
    return UGraph::StaticClass();
}

void FAssetTypeActions_Graph::OpenAssetEditor(const TArray<UObject *> &InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
    const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

    for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
    {
        if (UGraph *Graph = Cast<UGraph>(*ObjIt))
        {
            TSharedRef<FGraphAssetEditor> NewGraphEditor(new FGraphAssetEditor());
            NewGraphEditor->InitGenericGraphAssetEditor(Mode, EditWithinLevelEditor, Graph);
        }
    }
}

uint32 FAssetTypeActions_Graph::GetCategories()
{
    return EAssetTypeCategories::Animation | MyAssetCategory;
}

#define LOCTEXT_NAMESPACE "GraphAssetTypeActions"