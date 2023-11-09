#include "FSMGraphAssetTypeActions.h"
#include "FSMGraph.h"
#include "Editor/FSMGraphEditor.h"

FText FAssetTypeActions_FSMGraph::GetName() const
{
    return FText::FromString("FSM Graph");
}

FColor FAssetTypeActions_FSMGraph::GetTypeColor() const
{
    return FColor(129,129,129);
}

UClass *FAssetTypeActions_FSMGraph::GetSupportedClass() const
{
    return UFSMGraph::StaticClass();
}

void FAssetTypeActions_FSMGraph::OpenAssetEditor(const TArray<UObject *> &InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
    EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

    for(auto Object : InObjects)
    {
        auto NewAsset = Cast<UFSMGraph>(Object);
        if(NewAsset != nullptr)
        {
            TSharedRef<FSMGraphAssetEditor> NewEditor = MakeShareable(new FSMGraphAssetEditor());
            NewEditor->InitGraphAssetEditor(Mode, EditWithinLevelEditor, NewAsset);
        }
    }
}

uint32 FAssetTypeActions_FSMGraph::GetCategories() 
{
    return EAssetTypeCategories::Blueprint;
}