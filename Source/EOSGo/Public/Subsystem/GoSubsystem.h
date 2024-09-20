// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GoSubsystem.generated.h"

//~ GO SUBSYSTEM DELEGATES
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnLoginComplete, FName, Username);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnUpdateSessionComplete, bool, bWasSuccess);
DECLARE_MULTICAST_DELEGATE_TwoParams(FGoOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool  bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FGoOnJoinSessionComplete, FName SessionName, EOnJoinSessionCompleteResult::Type Result);
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
	UPROPERTY(BlueprintAssignable, Category="EOS-Go|Account")
	FGoOnLoginComplete GoOnLoginComplete;
	
	UFUNCTION(BlueprintPure, Category="EOS-Go|Account")
	bool IsPlayerLoggedIn();
	UFUNCTION(BlueprintPure, Category="EOS-Go|Account")
	FName GetPlayerUsername();

	
	//~ To handle session functionality.
	void GoCreateSession(int32 NumberOfConnections, FString MatchType, int32 ServerPrivateJoinId, bool bIsPrivateSession);
	FGoOnCreateSessionComplete GoOnCreateSessionComplete;
	void UpdateSession(FOnlineSessionSettings& UpdateSessionSettings);
	FGoOnUpdateSessionComplete GoOnUpdateSessionComplete;
	void GoFindSessions(int64 InServerJoinId);
	FGoOnFindSessionsComplete GoOnFindSessionsComplete;
	void GoJoinSession(const FOnlineSessionSearchResult& SessionSearchResult);
	FGoOnJoinSessionComplete GoOnJoinSessionComplete;
	void GoDestroySession();
	FGoOnDestroySessionComplete GoOnDestroySessionComplete;
	void GoStartSession();
	FGoOnStartSessionComplete GoOnStartSessionComplete;

	UPROPERTY(BlueprintReadWrite)
	int32 ServerJoinId = 0;	//~ Server Join Id displayed on the UI.
	
protected:
	//~ To handle Login functionality.
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error);
	
	//~ To handle session functionality.
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccess);
	void OnUpdateSessionComplete(FName SessionName, bool bWasSuccess);
	void OnFindSessionsComplete(bool bWasSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccess);
	void OnStartSessionComplete(FName SessionName,bool bWasSuccess);
	
private:
	IOnlineIdentityPtr Identity;
	IOnlineUserPtr UserInterface;
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineUser> User;
	TSharedPtr<FOnlineSessionSearch> SessionSearchSettings;

	//~ Delegates to add to the Online Session Interface delegate list. Each one has its own handle.
	FOnLoginCompleteDelegate LoginCompleteDelegate;
	FDelegateHandle LoginCompleteDelegateHandle;
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnUpdateSessionCompleteDelegate UpdateSessionCompleteDelegate;
	FDelegateHandle UpdateSessionCompleteDelegateHandle;
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
	int64 LastServerPrivateJoinId = 0;
	FString LastMatchType {FString()};
	
	//~ Persistent Data
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FName LoggedPlayerUsername{"Unknown"};

};
