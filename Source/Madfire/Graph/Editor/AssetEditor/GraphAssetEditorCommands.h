#pragma once

#include "CoreMinimal.h"

class FGraphAssetEditorCommands : public TCommands<FGraphAssetEditorCommands>
{
public:
    FGraphAssetEditorCommands() : 
        TCommands<FGraphAssetEditorCommands>("GraphEditor", NSLOCTEXT("Contexts", "GraphEditor", "Graph Editor"), NAME_None, FAppStyle::GetAppStyleSetName())
    {}

    TSharedPtr<FUICommandInfo> GraphSettings;
    TSharedPtr<FUICommandInfo> AutoArrange;

    void RegisterCommands() override;
};