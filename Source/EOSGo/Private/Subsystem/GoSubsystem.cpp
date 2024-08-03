// Fill out your copyright notice in the Description page of Project Settings.


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
	bWasSuccess ? LogMessage(FColor::Green,FString(TEXT("Login Successful"))) : LogMessage(FColor::Red,FString(TEXT("Login Failed")));
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
	GoOnCreateSessionComplete.Broadcast(bWasSuccess);
}
void UGoSubsystem::GoCreateSession(int32 NumberOfPublicConnections, FString MatchType)
{
	if (!SessionInterface.IsValid()) return;

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	//~ If same named session exists, it will be deleted.
	if (ExistingSession != nullptr) SessionInterface->DestroySession(NAME_GameSession);
	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	//~ Set session settings
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsDedicated = false;
	LastSessionSettings->bIsLANMatch = false;
	LastSessionSettings->NumPublicConnections = NumberOfPublicConnections;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bAllowInvites = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->bUseLobbiesVoiceChatIfAvailable = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	//~ CREATE
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,*LastSessionSettings))
	{
		//~ If it doesn't create the session, clear delegate of the delegate list.
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		//~ Broadcast Go Subsystem Delegate - Creation not successful .
		GoOnCreateSessionComplete.Broadcast(false);
	}
}


void UGoSubsystem::OnFindSessionsComplete(bool bWasSuccess)
{
	//~ If searching was successful, clear delegate of the delegate list.
	if (SessionInterface) SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

	//~ Broadcast Go Subsystem Delegate - Searching successful.
	if (LastSessionSearch->SearchResults.Num() > 0)
	{
		GoOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccess); // 1+ sessions were found.
	}
	else
	{
		GoOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false); // No sessions were found.
	}
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
	LastSessionSearch->QuerySettings.Set(FName("SEARCH_PRESENCE"), true, EOnlineComparisonOp::Equals);

	//~ SEARCH
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetCachedUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
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
	GoOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::Success);
}
void UGoSubsystem::GoJoinSession(const FOnlineSessionSearchResult& SessionSearchResult)
{
	if (!SessionInterface.IsValid())
	{	
		GoOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}
	
	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

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
	
}
void UGoSubsystem::GoDestroySession()
{
	
}


void UGoSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccess)
{
	
}
void UGoSubsystem::GoStartSession()
{
	
}









