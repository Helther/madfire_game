#pragma once

#include "ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(FSMGraph, Log, All);

class FFSMGraphModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

	static FFSMGraphModule &Get()
	{
		return FModuleManager::LoadModuleChecked<FFSMGraphModule>("FSMGraphModule");
	}

	static bool IsAvailable() 
	{
		return FModuleManager::Get().IsModuleLoaded("FSMGraphEditor");
	}
};
