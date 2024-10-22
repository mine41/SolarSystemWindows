// Copyright Ilgar Lunin. All Rights Reserved.

#include "VoskPlugin.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FVoskPluginModule"

void FVoskPluginModule::StartupModule()
{
    FWebSocketsModule& Module = FModuleManager::LoadModuleChecked<FWebSocketsModule>(TEXT("WebSockets"));

}

void FVoskPluginModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVoskPluginModule, VoskPlugin)
