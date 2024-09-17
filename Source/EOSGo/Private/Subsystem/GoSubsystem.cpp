// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.

#include "Subsystem/GoSubsystem.h"
#include "EOSGo.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Engine/LocalPlayer.h"
#include "TimerManager.h"
#include "Engine/World.h"

UGoSubsystem::UGoSubsystem() :
/*Bind Delegates*/
LoginCompleteDelegate(FOnLoginCompleteDelegate::CreateUObject(this,&ThisClass::OnLoginComplete)),
CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
UpdateSessionCompleteDelegate(FOnUpdateSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnUpdateSessionComplete)),
FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		LogMessage("Subsystem is loaded!");
		Identity = Subsystem->GetIdentityInterface();
		SessionInterface = Subsystem->GetSessionInterface();
	}
}


void UGoSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error)
{
	//~ If Login was successful, clear delegate of the delegate list.
	if (Identity) Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginCompleteDelegateHandle);

	//~ Broadcast Go Subsystem Delegate - Login was successful.
	if (Identity->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
	{
		LoggedPlayerUsername = FName(Identity->GetPlayerNickname(UserId));
		GoOnLoginComplete.Broadcast(LoggedPlayerUsername);
		LogMessage("Login Successful");
	}
}
void UGoSubsystem::GoEOSLogin(FString Id, FString Token, FString LoginType)
{
	if(!Identity.IsValid()) return;

	//~ Player validations.
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
	{
		LogMessage("LocalPlayer on searching is invalid!");
		GoOnLoginComplete.Broadcast(FName("Unknown"));
		return;
	}

	//~ Get Player Local User Number.
	int32 LocalUserNumber = LocalPlayer->GetControllerId();

	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	LoginCompleteDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(LocalUserNumber, LoginCompleteDelegate);

	//~ Set Account Credentials.
	FOnlineAccountCredentials AccountDetails;
	AccountDetails.Id = Id;
	AccountDetails.Token = Token;
	AccountDetails.Type = LoginType;
	
	//~ LOGIN
	if (!Identity->Login(LocalUserNumber, AccountDetails))
	{
		LogMessage("Login Failed");
		//~ If Login wasn't successful, clear delegate of the delegate list.
		Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNumber, LoginCompleteDelegateHandle);
		//~ Broadcast Go Subsystem Delegate - Login wasn't successful.
		GoOnLoginComplete.Broadcast(FName("Unknown"));
	}
}


bool UGoSubsystem::IsPlayerLoggedIn()
{
	if(!Identity.IsValid()) return false;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	return Identity->GetLoginStatus(LocalPlayer->GetControllerId()) == ELoginStatus::LoggedIn;
}
FName UGoSubsystem::GetPlayerUsername()
{
	return LoggedPlayerUsername;
}


void UGoSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccess)
{
	//~ If session was created, clear delegate of the delegate list.
	if (SessionInterface) SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	
	//~ Broadcast Go Subsystem Delegate - Creation successful.
	GoOnCreateSessionComplete.Broadcast(true);
}
void UGoSubsystem::GoCreateSession(int32 NumberOfConnections, FString MatchType, int32 ServerPrivateJoinId, bool bIsPrivateSession)
{
    if (!SessionInterface.IsValid()) return;
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
	{
		LogMessage("LocalPlayer on creation is invalid!");
		GoOnCreateSessionComplete.Broadcast(false);
		return;
	}
	
    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	//~ If same named session exists, it will be deleted.
    if (ExistingSession != nullptr)
    {
        bCreateSessionOnDestroy = true;
        bCreatePrivateSession = bIsPrivateSession;
        LastServerPrivateJoinId = ServerPrivateJoinId;
        LastNumberOfPublicConnections = NumberOfConnections;
        LastMatchType = MatchType;
        GoDestroySession();
    }

	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	//~ Set session settings.
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShared<FOnlineSessionSettings>();
	SessionSettings->bIsDedicated = false;
	SessionSettings->bIsLANMatch = false;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bUseLobbiesIfAvailable = false;
	SessionSettings->bUseLobbiesVoiceChatIfAvailable = false;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesStats = true;
	SessionSettings->BuildUniqueId = 1;
	SessionSettings->Set(FName("MATCH_TYPE"), MatchType, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings->Set(FName("SERVER_IS_PRIVATE"), bIsPrivateSession, EOnlineDataAdvertisementType::ViaOnlineService);

	//~ Checks if Private Session was toggled and sets a Server Join Id to access to this session.
    if (bIsPrivateSession)
    {
        ServerJoinId = ServerPrivateJoinId;
        SessionSettings->Set(FName("SERVER_JOIN_ID"), ServerPrivateJoinId, EOnlineDataAdvertisementType::ViaOnlineService);
    	SessionSettings->NumPrivateConnections = NumberOfConnections;
    }
    else
    {
    	SessionSettings->Set(FName("SERVER_JOIN_ID"), 0, EOnlineDataAdvertisementType::ViaOnlineService);
    	SessionSettings->NumPublicConnections = NumberOfConnections;
    }
	
	//~ UniqueNetId& required to hosting the session.
	FUniqueNetIdRepl UniqueNetIdRepl = LocalPlayer->GetUniqueNetIdForPlatformUser();
    TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
    if (!UniqueNetId.IsValid())
    {
        LogMessage("UniqueNetId on creation is invalid!");
        GoOnCreateSessionComplete.Broadcast(false);
        return;
    }
	
	//~ CREATE
    if (!SessionInterface->CreateSession(*UniqueNetId, NAME_GameSession, *SessionSettings))
    {
    	//~ If it doesn't create the session, clear delegate of the delegate list.
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    	//~ Broadcast Go Subsystem Delegate - Creation not successful.
        GoOnCreateSessionComplete.Broadcast(false);
        ServerJoinId = 0;
    }
}


void UGoSubsystem::OnUpdateSessionComplete(FName SessionName, bool bWasSuccess)
{
	//~ If session was updated, clear delegate of the delegate list.
	if (SessionInterface) SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);

	//~ Broadcast Go Subsystem Delegate - Updating successful.
	GoOnUpdateSessionComplete.Broadcast(true);
	LogMessage("Session updated successfuly");
}
void UGoSubsystem::UpdateSession(FOnlineSessionSettings& UpdateSessionSettings)
{
	if (!SessionInterface.IsValid()) return;
	
	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	UpdateSessionCompleteDelegateHandle = SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegate);
	
	//~ UPDATE
	if (!SessionInterface->UpdateSession(NAME_GameSession, UpdateSessionSettings))
	{
		LogMessage("Updating Failed");
		//~ If Updating wasn't successful, clear delegate of the delegate list.
		SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
		//~ Broadcast Go Subsystem Delegate - Updating wasn't successful.
		GoOnUpdateSessionComplete.Broadcast(false);
	}
}


void UGoSubsystem::OnFindSessionsComplete(bool bWasSuccess)
{
	//~ If searching was successful, clear delegate of the delegate list.
	if (SessionInterface) SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

	//~ Use a timer to delay broadcasting the results.
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		if (SessionSearchSettings->SearchState == EOnlineAsyncTaskState::Done)
		{
			//~ Broadcast Go Subsystem Delegate - Searching successful.
			GoOnFindSessionsComplete.Broadcast(SessionSearchSettings->SearchResults, true); 
			LogMessage("EOnlineAsyncTaskState::Done");
			return;
		}
		if (SessionSearchSettings->SearchState == EOnlineAsyncTaskState::Failed)
		{
			//~ Broadcast Go Subsystem Delegate - Searching wasn't successful.
			GoOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
			LogMessage("EOnlineAsyncTaskState::Failed");
			return;
		}

		//~ Search is In Progress or Not Started
		LogMessage("Search is In Progress or Not Started");
	}, 3.0f, false);
}
void UGoSubsystem::GoFindSessions(int64 InServerJoinId)
{
	if (!SessionInterface.IsValid()) return;

	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	//~ Filter by
	SessionSearchSettings = MakeShareable(new FOnlineSessionSearch());
	SessionSearchSettings->QuerySettings.SearchParams.Empty();
	SessionSearchSettings->MaxSearchResults = 100;
	SessionSearchSettings->bIsLanQuery = false;
	//~ Add the attribute in order to join private sessions.
	SessionSearchSettings->QuerySettings.SearchParams.Add(
		FName("SERVER_JOIN_ID"), FOnlineSessionSearchParam(InServerJoinId, EOnlineComparisonOp::Equals)
	);
	
	//~ Player validations
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
	{
		LogMessage("LocalPlayer on searching is invalid!");
		GoOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
		return;
	}

	//~ UniqueNetId& required to search sessions.
	FUniqueNetIdRepl UniqueNetIdRepl = LocalPlayer->GetUniqueNetIdForPlatformUser();
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	
	if (!UniqueNetId.IsValid())
	{
		LogMessage("UniqueNetId on searching is invalid!");
		GoOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
		return;
	}
	
	//~ SEARCH
	if (!SessionInterface->FindSessions(*UniqueNetId, SessionSearchSettings.ToSharedRef()))
	{
		LogMessage("Searching for sessions failed");
		//~ If searching wasn't successful, clear delegate of the delegate list.
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		//~ Broadcast Go Subsystem Delegate - Searching wasn't successful.
		GoOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
	}
}


void UGoSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	//~ If joining was successful, clear delegate of the delegate list.
	if (SessionInterface) SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	
	//~ Broadcast Go Subsystem Delegate - Joining was successful.
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		GoOnJoinSessionComplete.Broadcast(SessionName, Result);
	}
}
void UGoSubsystem::GoJoinSession(const FOnlineSessionSearchResult& SessionSearchResult)
{
	if (!SessionInterface.IsValid())
	{	
		GoOnJoinSessionComplete.Broadcast(FName(), EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	//~ Player validations
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
	{
		LogMessage("LocalPlayer on join is invalid!");
		GoOnJoinSessionComplete.Broadcast(FName(), EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	//~ UniqueNetId& required to search sessions.
	FUniqueNetIdRepl UniqueNetIdRepl = LocalPlayer->GetUniqueNetIdForPlatformUser();
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	
	if (!UniqueNetId.IsValid())
	{
		LogMessage("UniqueNetId on join is invalid!");
		GoOnJoinSessionComplete.Broadcast(FName(), EOnJoinSessionCompleteResult::UnknownError);
		return;
	}
	
	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	//~ JOIN
	if (!SessionInterface->JoinSession(*UniqueNetId, NAME_GameSession, SessionSearchResult))
	{
		LogMessage("Joining Failed");
		//~ If joining wasn't successful, clear delegate of the delegate list.
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		//~ Broadcast Go Subsystem Delegate - Joining wasn't successful.
		GoOnJoinSessionComplete.Broadcast(FName(), EOnJoinSessionCompleteResult::UnknownError);
	}
}


void UGoSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccess)
{
	//~ If destroying was successful, clear delegate of the delegate list.
	if (SessionInterface) SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	
	//~ Broadcast Go Subsystem Delegate - Destroying was successful.
	if (bWasSuccess && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		GoOnDestroySessionComplete.Broadcast(bWasSuccess);
		GoCreateSession(LastNumberOfPublicConnections, LastMatchType, LastServerPrivateJoinId, bCreatePrivateSession);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Destroying session: %s "), *SessionName.ToString());
	GoOnDestroySessionComplete.Broadcast(bWasSuccess);
}
void UGoSubsystem::GoDestroySession()
{
	if (!SessionInterface.IsValid())
	{	
		GoOnDestroySessionComplete.Broadcast(false);
		return;
	}

	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	
	//~ DESTROY
	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		LogMessage("Destroy session Failed");
		//~ If destroying wasn't successful, clear delegate of the delegate list.
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		//~ Broadcast Go Subsystem Delegate - Destroying wasn't successful.
		GoOnDestroySessionComplete.Broadcast(false);
	}
}


void UGoSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccess)
{
	
}
void UGoSubsystem::GoStartSession()
{
	
}
