// Copyright Fedahumada - All Rights Reserved.

#include "UI/GoMenu.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"
#include "Subsystem/GoSubsystem.h"

bool UGoMenu::Initialize()
{
	if (!Super::Initialize()) return false;

	if (Host_Button)
	{
		Host_Button->OnClicked.AddDynamic(this, &UGoMenu::HostButtonClicked);
	}
	if (Join_Button)
	{
		Join_Button->OnClicked.AddDynamic(this, &UGoMenu::JoinButtonClicked);
	}
	if (Login_Button)
	{
		Login_Button->OnClicked.AddDynamic(this, &UGoMenu::LoginButtonClicked);
	}
	return true;
}

void UGoMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UGoMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LevelPath)
{
	MapToTravel = LevelPath;
	MatchType = TypeOfMatch;
	NumberOfConnections = NumberOfPublicConnections;
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
		GoSubsystem->GoOnCreateSessionComplete.AddDynamic(this, &UGoMenu::OnCreateSession);
		GoSubsystem->GoOnFindSessionsComplete.AddUObject(this, &UGoMenu::OnFindSessions);
		GoSubsystem->GoOnJoinSessionComplete.AddUObject(this, &UGoMenu::OnJoinSession);
		GoSubsystem->GoOnDestroySessionComplete.AddDynamic(this, &UGoMenu::OnDestroySession);
		GoSubsystem->GoOnStartSessionComplete.AddDynamic(this, &UGoMenu::OnStartSession);
	}
}

void UGoMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		LogMessage(FString("Session created successfully"));
		if (UWorld* World = GetWorld()) World->ServerTravel(MapToTravel);
	}
	else
	{
		LogMessage(FString("Failed creating session"));
		Host_Button->SetIsEnabled(true);
	}
}

void UGoMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (!IsValid(GoSubsystem)) return;
	if (SessionResults.IsEmpty()) LogMessage(FString("No sessions found!"));

	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MATCH_TYPE"), SettingsValue);
		if (SettingsValue == MatchType)
		{
			GoSubsystem->GoJoinSession(Result);
			return;
		}
	}

	if (!bWasSuccessful || SessionResults.IsEmpty())
	{
		Join_Button->SetIsEnabled(true);	
	}
}

void UGoMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
			if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
			{
				LogMessage(FString(TEXT("Traveling...")));
				PlayerController->ClientTravel(Address, TRAVEL_Absolute);
			}
		}
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		Join_Button->SetIsEnabled(true);
	}
}

void UGoMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UGoMenu::OnStartSession(bool bWasSuccessful)
{
}

void UGoMenu::HostButtonClicked()
{
	Host_Button->SetIsEnabled(false);
	if (GoSubsystem) GoSubsystem->GoCreateSession(NumberOfConnections, MatchType);
}

void UGoMenu::JoinButtonClicked()
{
	Join_Button->SetIsEnabled(false);
	if (GoSubsystem) GoSubsystem->GoFindSessions(100);
}

void UGoMenu::LoginButtonClicked()
{
	if (GoSubsystem) GoSubsystem->GoEOSLogin("","","accountportal");
}

void UGoMenu::MenuTearDown()
{
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
