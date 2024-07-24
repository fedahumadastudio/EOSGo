// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/GoSubsystem.h"

#include "OnlineSubsystem.h"

void UGoSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

UGoSubsystem::UGoSubsystem()
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
}
