#pragma once

#include "CoreMinimal.h"

class FGraphAssetEditor;
class FExtender;
class FToolBarBuilder;

class FGraphAssetEditroToolbar : public TSharedFromThis<FGraphAssetEditroToolbar>
{
public:
    FGraphAssetEditroToolbar(TSharedPtr<FGraphAssetEditor> InGenericGraphEditor) : 
        GraphEditor(InGenericGraphEditor)
    {}

    void AddGenericGraphToolbar(TSharedPtr<FExtender> Extender);

private:
    void FillGraphToolbar(FToolBarBuilder &ToolbarBuilder);

protected:
    TWeakPtr<FGraphAssetEditor> GraphEditor;
};