// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FEOSGoModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
};

static void LogMessage(FString Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}
