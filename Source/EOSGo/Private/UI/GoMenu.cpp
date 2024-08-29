// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.

#include "UI/GoMenu.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Subsystem/GoSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include"Kismet/KismetSystemLibrary.h"

bool UGoMenu::Initialize()
{
	
	if (!Super::Initialize()) return false;

	if (Login_Button)
	{
		Login_Button->OnClicked.AddDynamic(this, &UGoMenu::LoginButtonClicked);
	}
	if (HostLobby_Button)
	{
		HostLobby_Button->OnClicked.AddDynamic(this, &UGoMenu::HostLobbyButtonClicked);
	}
	if (JoinLobby_Button)
	{
		JoinLobby_Button->OnClicked.AddDynamic(this, &UGoMenu::JoinLobbyButtonClicked);
	}
	if (Quit_Button)
	{
		Quit_Button->OnClicked.AddDynamic(this, &UGoMenu::QuitButtonClicked);
	}
	return true;
}

void UGoMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UGoMenu::GoMenuSetup(FString LobbyMapPath)
{
	LobbyMap = LobbyMapPath;
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
	}
}

void UGoMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		LogMessage("Session created successfully!");
		if (UWorld* World = GetWorld()) World->ServerTravel(LobbyMap);
	}
	else
	{
		LogMessage("Failed creating session!");
		HostLobby_Button->SetIsEnabled(true);
	}
}

void UGoMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (!IsValid(GoSubsystem)) return;
	if (SessionResults.IsEmpty()) LogMessage("No sessions found!");

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
		JoinLobby_Button->SetIsEnabled(true);	
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
				LogMessage("Traveling...");
				PlayerController->ClientTravel(Address, TRAVEL_Absolute);
			}
		}
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinLobby_Button->SetIsEnabled(true);
	}
}

void UGoMenu::LoginButtonClicked()
{
	if (!IsValid(GoSubsystem)) return;
	
	FString LoginType = "";
	FString Token = "";
	FString Id = "";
	
	if (FParse::Value(FCommandLine::Get(), TEXT("-AUTH_TYPE="), LoginType))
	{
		if (FParse::Value(FCommandLine::Get(), TEXT("-AUTH_TOKEN="), Token))
		{
			if (FParse::Value(FCommandLine::Get(), TEXT("-AUTH_ID="), Id))
			{
				LogMessage("Login Auth Data Retrived: AUTH_TYPE=" + LoginType + " | AUTH_TOKEN=" + Token + " | AUTH_ID=" + Id);
			}
		}
	}
	
	if (LoginType != "" && Token != "" && Id != "")
	{
		GoSubsystem->GoEOSLogin(Id,Token,LoginType);
	}
	else
	{
		GoSubsystem->GoEOSLogin("","","accountportal");
	}
}

void UGoMenu::HostLobbyButtonClicked()
{
	HostLobby_Button->SetIsEnabled(false);
	ServerPrivateJoinId = FMath::RandRange(1111,99999);
	if (GoSubsystem) GoSubsystem->GoCreateSession(NumberOfConnections, MatchType, ServerPrivateJoinId, bIsPrivate);
}

void UGoMenu::JoinLobbyButtonClicked()
{
	JoinLobby_Button->SetIsEnabled(false);
	if (GoSubsystem) GoSubsystem->GoFindSessions(100, ServerPrivateJoinId);
}

void UGoMenu::QuitButtonClicked()
{
	if (GoSubsystem) UKismetSystemLibrary::QuitGame(GetWorld(),UGameplayStatics::GetPlayerController(GetWorld(),0),EQuitPreference::Type::Quit,false);
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
