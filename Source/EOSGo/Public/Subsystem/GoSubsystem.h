// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GoSubsystem.generated.h"

//~ GO SUBSYSTEM DELEGATES
							//For GoMenu class to bind callbacks to.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnLoginComplete, FName, Username);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FGoOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool  bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FGoOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnStartSessionComplete, bool, bWasSuccessful);
							//For GoOverlay class to bind callbacks to.
//DECLARE_MULTICAST_DELEGATE_TwoParams(FGoOnReadFriendsListComplete, TArray<TSharedRef<FOnlineFriend>> FriendsArray, bool bWasSuccessful);

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
	UPROPERTY(BlueprintAssignable, Category="EOS-Go|Account")
	FGoOnLoginComplete GoOnLoginComplete;
	
	UFUNCTION(BlueprintPure, Category="EOS-Go|Account")
	bool IsPlayerLoggedIn();
	UFUNCTION(BlueprintPure, Category="EOS-Go|Account")
	FName GetPlayerUsername();

	
	//~ To handle session functionality.
	void GoCreateSession(int32 NumberOfPublicConnections, FString MatchType, int32 ServerPrivateJoinId, bool bIsPrivateSession);
	FGoOnCreateSessionComplete GoOnCreateSessionComplete;
	void GoFindSessions(int32 MaxSearchResults, int32 ServerPrivateJoinId);
	FGoOnFindSessionsComplete GoOnFindSessionsComplete;
	void GoJoinSession(const FOnlineSessionSearchResult& SessionSearchResult);
	FGoOnJoinSessionComplete GoOnJoinSessionComplete;
	void GoDestroySession();
	FGoOnDestroySessionComplete GoOnDestroySessionComplete;
	void GoStartSession();
	FGoOnStartSessionComplete GoOnStartSessionComplete;
	
	//~ To handle Friends functionality.
	//void GoReadFriendsList();
	//FGoOnReadFriendsListComplete GoOnReadFriendsListComplete;

protected:
	//~ To handle Login functionality.
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error);
	
	//~ To handle session functionality.
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccess);
	void OnFindSessionsComplete(bool bWasSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccess);
	void OnStartSessionComplete(FName SessionName,bool bWasSuccess);

	//~ To handle Friends functionality.
	//void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
	
private:
	IOnlineIdentityPtr Identity;
	IOnlineSessionPtr SessionInterface;
	IOnlineFriendsPtr FriendsInterface;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	TArray<TSharedRef<FOnlineFriend>> FriendsList;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	int32 PublicServerJoinId{0};

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
	bool bCreateSessionOnDestroy {false};
	bool bCreatePrivateSession {false};
	int32 LastNumberOfPublicConnections = 0;
	int32 LastServerPrivateJoinId = 0;
	FString LastMatchType;

	//~ Persistent Data
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FName LoggedPlayerUsername{"Unknown"};

};

static void LogMessage(FString Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}
