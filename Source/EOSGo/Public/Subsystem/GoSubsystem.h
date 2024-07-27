// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GoSubsystem.generated.h"

//~ GO SUBSYSTEM DELEGATES - For GoMenu class to bind callbacks to.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FGoOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool  bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FGoOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnStartSessionComplete, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class EOSGO_API UGoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UGoSubsystem();

	//~ To handle session functionality. GoMenu class will call these.
	void GoCreateSession(int32 NumberOfPublicConnections, FString MatchType);
	FGoOnCreateSessionComplete GoOnCreateSessionComplete;
	void GoFindSessions(int32 MaxSearchResults);
	FGoOnFindSessionsComplete GoOnFindSessionsComplete;
	void GoJoinSession(const FOnlineSessionSearchResult& SessionSearchResult);
	FGoOnJoinSessionComplete GoOnJoinSessionComplete;
	void GoDestroySession();
	FGoOnDestroySessionComplete GoOnDestroySessionComplete;
	void GoStartSession();
	FGoOnStartSessionComplete GoOnStartSessionComplete;

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccess);
	void OnFindSessionsComplete(bool bWasSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccess);
	void OnStartSessionComplete(FName SessionName,bool bWasSuccess);
	
private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	//~ Delegates to add to the Online Session Interface delegate list. Each one has its own handle.
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;
};

static void ScreenMessage(FColor Color, FString Message)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1,10.f,Color, *Message);
}
