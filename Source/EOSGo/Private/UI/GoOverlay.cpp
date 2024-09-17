// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.

#include "UI/GoOverlay.h"
#include "EOSGo.h"
#include "Subsystem/GoSubsystem.h"
#include "Components/Button.h"
#include "Game/GoGameModeBase.h"

bool UGoOverlay::Initialize()
{
	if (!Super::Initialize()) return false;

	if (ExitSession_Button)
	{
		ExitSession_Button->OnClicked.AddDynamic(this, &UGoOverlay::ExitSessionButtonClicked);
	}
	return true;
}

void UGoOverlay::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UGoOverlay::GoOverlaySetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		GoSubsystem = GameInstance->GetSubsystem<UGoSubsystem>();
	}
	
	if (IsValid(GoSubsystem))
	{
		//~ Bind session callbacks.
		GoSubsystem->GoOnDestroySessionComplete.AddDynamic(this, &UGoOverlay::OnDestroySession);
		GoSubsystem->GoOnStartSessionComplete.AddDynamic(this, &UGoOverlay::OnStartSession);
	}
}


void UGoOverlay::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		LogMessage("Failed destroying session!");
		ExitSession_Button->SetIsEnabled(true);
		return;
	}
	
	if (UWorld* World = GetWorld())
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode())
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
	LogMessage("Session destroyed successfully!");
}

void UGoOverlay::OnStartSession(bool bWasSuccessful)
{
}

void UGoOverlay::ExitSessionButtonClicked()
{
	ExitSession_Button->SetIsEnabled(false);

	//~ Call destroy session
	if (GoSubsystem) GoSubsystem->GoDestroySession();
}

void UGoOverlay::MenuTearDown()
{
	//~ Call destroy session
	if (GoSubsystem) GoSubsystem->GoDestroySession();
	
	RemoveFromParent();
	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			const FInputModeGameOnly InputData;
			PlayerController->SetInputMode(InputData);
		}
	}
}
