// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.

#include "UI/GoMenu.h"
#include "EOSGo.h"
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

	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
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
	//~ Validations
	if (!IsValid(GoSubsystem)) return;
	if (!bWasSuccessful)
	{
		LogMessage("Search was not successful!");
		JoinLobby_Button->SetIsEnabled(true);
		return;
	}
	if (SessionResults.IsEmpty())
	{
		LogMessage("No sessions found!");
		JoinLobby_Button->SetIsEnabled(true);
		return;
	}
	
	//~ Session Results Filter & Join
	for (const auto& Result : SessionResults)
	{
		GoSubsystem->GoJoinSession(Result);
		return;
	}
}

void UGoMenu::OnJoinSession(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	//~ Validations
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinLobby_Button->SetIsEnabled(true);
		return;
	}
	if (!SessionInterface.IsValid()) 
	{
		LogMessage("Invalid Session Interface!");
		return;
	}

	//~ TRAVEL
	FString ConnectionInfo;
	SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectionInfo);
	
	if (!ConnectionInfo.IsEmpty())
	{
		if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
		{
			LogMessage("Traveling...");
			PlayerController->ClientTravel(ConnectionInfo, TRAVEL_Absolute);
			return;
		}
		LogMessage("Player could not travel. ClientTravel Failed!");
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

	//~ Call Login 
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
	ServerJoinId = FMath::RandRange(10000,99999);

	//~ Call create session
	if (GoSubsystem) GoSubsystem->GoCreateSession(NumberOfConnections, MatchType, ServerJoinId, bIsPrivate);
}

void UGoMenu::JoinLobbyButtonClicked()
{
	JoinLobby_Button->SetIsEnabled(false);

	//~ Call find sessions
	if (GoSubsystem) GoSubsystem->GoFindSessions(ServerJoinId);
	ServerJoinId = 0;
}

void UGoMenu::QuitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(),UGameplayStatics::GetPlayerController(GetWorld(),0),EQuitPreference::Type::Quit,false);
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
