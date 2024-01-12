// Copyright Epic Games, Inc. All Rights Reserved.

#include "BrainCloudS2SPlugin.h"

#define LOCTEXT_NAMESPACE "FBrainCloudS2SPluginModule"

void FBrainCloudS2SPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FBrainCloudS2SPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBrainCloudS2SPluginModule, BrainCloudS2SPlugin)

DEFINE_LOG_CATEGORY(LogBrainCloudS2S);
DEFINE_LOG_CATEGORY(S2SWebSocket);