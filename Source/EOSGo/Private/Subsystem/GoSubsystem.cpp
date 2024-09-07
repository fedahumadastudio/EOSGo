// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.

#include "Subsystem/GoSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Engine/LocalPlayer.h"

UGoSubsystem::UGoSubsystem() :
/*Bind Delegates*/
CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		Identity = Subsystem->GetIdentityInterface();
		SessionInterface = Subsystem->GetSessionInterface();
		FriendsInterface = Subsystem->GetFriendsInterface();
	}
}


void UGoSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error)
{
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (Identity->GetLoginStatus(LocalPlayer->GetControllerId()) == ELoginStatus::LoggedIn)
	{
		LoggedPlayerUsername = FName(Identity->GetPlayerNickname(LocalPlayer->GetControllerId()));
		LogMessage("Login Successful");
	}
	GoOnLoginComplete.Broadcast(LoggedPlayerUsername);
	
}
void UGoSubsystem::GoEOSLogin(FString Id, FString Token, FString LoginType)
{
	if(!Identity.IsValid()) return;

	FOnlineAccountCredentials AccountDetails;
	AccountDetails.Id = Id;
	AccountDetails.Token = Token;
	AccountDetails.Type = LoginType;

	//~ Bind login callback.
	Identity->OnLoginCompleteDelegates->AddUObject(this, &UGoSubsystem::OnLoginComplete);

	//~ LOGIN
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!Identity->Login(LocalPlayer->GetControllerId(), AccountDetails))
	{
		LogMessage("Login Failed");
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

	//~ Broadcast Go Subsystem Delegate - Creation successful .
	if (bWasSuccess)
	{
		GoOnCreateSessionComplete.Broadcast(bWasSuccess);
	}
}
void UGoSubsystem::GoCreateSession(int32 NumberOfPublicConnections, FString MatchType, int32 ServerPrivateJoinId, bool bIsPrivateSession)
{
    if (!SessionInterface.IsValid()) return;

    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	//~ If same named session exists, it will be deleted.
    if (ExistingSession != nullptr)
    {
        bCreateSessionOnDestroy = true;
        bCreatePrivateSession = bIsPrivateSession;
        LastServerPrivateJoinId = ServerPrivateJoinId;
        LastNumberOfPublicConnections = NumberOfPublicConnections;
        LastMatchType = MatchType;
        GoDestroySession();
    }

	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	//~ Set session settings
    TSharedPtr<FOnlineSessionSettings> LastSessionSettings = MakeShared<FOnlineSessionSettings>();
    LastSessionSettings->bIsDedicated = false;
    LastSessionSettings->bIsLANMatch = false;
    LastSessionSettings->NumPublicConnections = NumberOfPublicConnections;
    LastSessionSettings->bUsesPresence = true;
    LastSessionSettings->bAllowJoinViaPresence = true;
    LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
    LastSessionSettings->bAllowInvites = true;
    LastSessionSettings->bAllowJoinInProgress = true;
    LastSessionSettings->bUseLobbiesIfAvailable = false;
    LastSessionSettings->bUseLobbiesVoiceChatIfAvailable = false;
    LastSessionSettings->bShouldAdvertise = true;
    LastSessionSettings->bUsesStats = true;
    LastSessionSettings->BuildUniqueId = 1;
    LastSessionSettings->Set(FName("MATCH_TYPE"), MatchType, EOnlineDataAdvertisementType::ViaOnlineService);
    LastSessionSettings->Set(FName("SERVER_IS_PRIVATE"), bIsPrivateSession, EOnlineDataAdvertisementType::ViaOnlineService);

	//~ Checks if Private Session was toggled and sets a Server Join Id to access to this session.
    if (bIsPrivateSession)
    {
        PublicServerJoinId = ServerPrivateJoinId;
        LastSessionSettings->Set(FName("SERVER_JOIN_ID"), ServerPrivateJoinId, EOnlineDataAdvertisementType::ViaOnlineService);
    }

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("LocalPlayer is invalid!"));
        GoOnCreateSessionComplete.Broadcast(false);
        return;
    }

	//~ UniqueNetId& required to hosting the session.
	FUniqueNetIdRepl UniqueNetIdRepl = LocalPlayer->GetUniqueNetIdForPlatformUser();
    TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
    if (!UniqueNetId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UniqueNetId is invalid!"));
        GoOnCreateSessionComplete.Broadcast(false);
        return;
    }

	//~ CREATE
    if (!SessionInterface->CreateSession(*UniqueNetId, NAME_GameSession, *LastSessionSettings))
    {
    	//~ If it doesn't create the session, clear delegate of the delegate list.
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    	//~ Broadcast Go Subsystem Delegate - Creation not successful.
        GoOnCreateSessionComplete.Broadcast(false);
        PublicServerJoinId = 0;
    }
}


void UGoSubsystem::OnFindSessionsComplete(bool bWasSuccess)
{
	//~ If searching was successful, clear delegate of the delegate list.
	if (SessionInterface) SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

	//~ Broadcast Go Subsystem Delegate - Searching successful.
	if (LastSessionSearch->SearchResults.IsEmpty())
	{
		GoOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false); // No sessions were found.
		return;
	}
	
	GoOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccess); // 1+ sessions were found.
}
void UGoSubsystem::GoFindSessions(int32 MaxSearchResults, int32 ServerPrivateJoinId)
{
	if (!SessionInterface.IsValid()) return;

	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	//~ Filter by
	
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = false;
	if (ServerPrivateJoinId > 0)
	{
		LastSessionSearch->QuerySettings.Set(FName("SERVER_IS_PRIVATE"), true, EOnlineComparisonOp::Equals);
		LastSessionSearch->QuerySettings.Set(FName("SERVER_JOIN_ID"), ServerPrivateJoinId, EOnlineComparisonOp::Equals);
	}
	else
	{
		LastSessionSearch->QuerySettings.Set(FName("SERVER_IS_PRIVATE"), false, EOnlineComparisonOp::Equals);
	}

	//~ SEARCH
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer && !SessionInterface->FindSessions(0, LastSessionSearch.ToSharedRef()))
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
		GoOnJoinSessionComplete.Broadcast(Result);
	}
}
void UGoSubsystem::GoJoinSession(const FOnlineSessionSearchResult& SessionSearchResult)
{
	if (!SessionInterface.IsValid())
	{	
		GoOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}
	
	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	//~ JOIN
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSearchResult))
	{
		//~ If joining wasn't successful, clear delegate of the delegate list.
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		//~ Broadcast Go Subsystem Delegate - Joining wasn't successful.
		GoOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
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

/*

void UGoSubsystem::OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (!bWasSuccessful) LogMessage("Error reading friends list. Error:" + ErrorStr);

	//~ Broadcast Go Subsystem Delegate - Searching successful.
	if (FriendsInterface) FriendsInterface->GetFriendsList(0,FString(""), FriendsList);
	
	if (FriendsList.IsEmpty())
	{
		LogMessage("Friends list empty.");
		GoOnReadFriendsListComplete.Broadcast(TArray<TSharedRef<FOnlineFriend>>(), bWasSuccessful);
		return;
	}

	GoOnReadFriendsListComplete.Broadcast(FriendsList, bWasSuccessful);
}
void UGoSubsystem::GoReadFriendsList()
{
	if (!FriendsInterface.IsValid()) return;

	//~ READ
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer && !FriendsInterface->ReadFriendsList(0, FString(""), FOnReadFriendsListComplete::CreateUObject(this, &UGoSubsystem::OnReadFriendsListComplete)))
	{
		LogMessage("Read friends list failed");
		//~ Broadcast Go Subsystem Delegate - Read friends list wasn't successful.
		GoOnReadFriendsListComplete.Broadcast(TArray<TSharedRef<FOnlineFriend>>(),false);
	}
}

*/
