// Copyright Fedahumada - All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineIdentityInterface.h"
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

	//~ To handle EOS login functionality.
	void GoEOSLogin(FString Id, FString Token, FString LoginType);
	void GoEOSLogin_Response(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error) const;

	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="EOS-Go|Account")
	bool IsPlayerLoggedIn();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="EOS-Go|Account")
	FString GetPlayerUsername();
	

	//~ To handle session functionality. GoMenu clicked buttons will call these.
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
	IOnlineIdentityPtr Identity;
	IOnlineSessionPtr SessionInterface;
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

	//~ OnDestroySession utils
	bool bCreateSessionOnDestroy { false };
	int32 LastNumberOfPublicConnections = 0;
	FString LastMatchType;
};

static void LogMessage(FString Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}
