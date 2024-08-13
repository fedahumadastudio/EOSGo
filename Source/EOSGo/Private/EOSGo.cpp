// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.

#include "EOSGo.h"

#define LOCTEXT_NAMESPACE "FEOSGoModule"

void FEOSGoModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FEOSGoModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEOSGoModule, EOSGo)