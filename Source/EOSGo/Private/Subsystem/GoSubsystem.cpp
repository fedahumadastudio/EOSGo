// Copyright Fedahumada - All Rights Reserved.

#include "Subsystem/GoSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"

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
	}
}

void UGoSubsystem::GoEOSLogin(FString Id, FString Token, FString LoginType)
{
	if(!Identity.IsValid()) return;

	FOnlineAccountCredentials AccountDetails;
	AccountDetails.Id = Id;
	AccountDetails.Token = Token;
	AccountDetails.Type = LoginType;

	//~ Bind login callback.
	Identity->OnLoginCompleteDelegates->AddUObject(this, &UGoSubsystem::GoEOSLogin_Response);

	//~ LOGIN
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	Identity->Login(LocalPlayer->GetControllerId(), AccountDetails);
	
}
void UGoSubsystem::GoEOSLogin_Response(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error) const
{
	bWasSuccess ? LogMessage(FString(TEXT("Login Successful"))) : LogMessage(FString(TEXT("Login Failed")));
}


bool UGoSubsystem::IsPlayerLoggedIn()
{
	if(!Identity.IsValid()) return false;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	return Identity->GetLoginStatus(LocalPlayer->GetControllerId()) == ELoginStatus::LoggedIn;
}
FString UGoSubsystem::GetPlayerUsername()
{
	if(!Identity.IsValid()) FString();

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (Identity->GetLoginStatus(LocalPlayer->GetControllerId()) == ELoginStatus::LoggedIn)
	{
		return Identity->GetPlayerNickname(LocalPlayer->GetControllerId());
	}
	return FString();
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
void UGoSubsystem::GoCreateSession(int32 NumberOfPublicConnections, FString MatchType)
{
	if (!SessionInterface.IsValid()) return;

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	//~ If same named session exists, it will be deleted.
	if (ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;	
		LastNumberOfPublicConnections = NumberOfPublicConnections;
		LastMatchType = MatchType;
		//~ DESTROY
		GoDestroySession();
	}
	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	//~ Set session settings
	TSharedRef<FOnlineSessionSettings> LastSessionSettings = MakeShared<FOnlineSessionSettings>();
	LastSessionSettings->bIsDedicated = false; 
	LastSessionSettings->bIsLANMatch = false;
	LastSessionSettings->NumPublicConnections = NumberOfPublicConnections; 
	LastSessionSettings->bUsesPresence = true;   //No presence on dedicated server. This requires a local user.
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
	LastSessionSettings->bAllowInvites = true;    //Allow inviting players into session. This requires presence and a local user. 
	LastSessionSettings->bAllowJoinInProgress = true; //Once the session is started, no one can join.
	LastSessionSettings->bUseLobbiesIfAvailable = false; 
	LastSessionSettings->bUseLobbiesVoiceChatIfAvailable = false; //We will also enable voice
	LastSessionSettings->bShouldAdvertise = true; //This creates a public match and will be searchable.
	LastSessionSettings->bUsesStats = true; //Needed to keep track of player stats.
	LastSessionSettings->BuildUniqueId = 1;
	LastSessionSettings->Set(FName("MATCH_TYPE"), MatchType, EOnlineDataAdvertisementType::ViaOnlineService);

	//~ CREATE
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,*LastSessionSettings))
	{
		//~ If it doesn't create the session, clear delegate of the delegate list.
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		//~ Broadcast Go Subsystem Delegate - Creation not successful.
		GoOnCreateSessionComplete.Broadcast(false);
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
void UGoSubsystem::GoFindSessions(int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid()) return;

	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	//~ Filter by
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = false;
	LastSessionSearch->QuerySettings.SearchParams.Empty();

	//~ SEARCH
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer && !SessionInterface->FindSessions(0, LastSessionSearch.ToSharedRef()))
	{
		LogMessage(FString("Searching for sessions failed"));
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
		GoCreateSession(LastNumberOfPublicConnections, LastMatchType);
	}
	
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









