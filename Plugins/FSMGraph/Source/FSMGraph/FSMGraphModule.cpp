#include "FSMGraphModule.h"
#include "AssetToolsModule.h"
#include "FSMGraphAssetTypeActions.h"

DEFINE_LOG_CATEGORY(FSMGraph);


void FFSMGraphModule::StartupModule()  
{
	UE_LOG(FSMGraph, Log, TEXT("FSM Module Start"));

	IAssetTools &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_FSMGraph));
}

void FFSMGraphModule::ShutdownModule() 
{
	UE_LOG(FSMGraph, Log, TEXT("FSM Module Shutdown"));
}


IMPLEMENT_MODULE(FFSMGraphModule, FSMGraphPluggin) 
