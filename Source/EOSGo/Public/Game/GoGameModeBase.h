// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "GameFramework/GameMode.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GoGameModeBase.generated.h"
class UGoSubsystem;

//~ GO GAME MODE DELEGATES
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnRegisterPlayerComplete, bool, bWasSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGoOnUnregisterPlayerComplete, bool, bWasSuccess);

/**
 * 
 */
UCLASS()
class EOSGO_API AGoGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGoGameModeBase();

	//~ To handle Session Login/out functionality.
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* ExitingPlayer) override;
	
	//~ To handle Registered Players functionality.
	void GoRegisterPlayer(const APlayerState* NewPlayerState);
	UPROPERTY(BlueprintAssignable, Category="EOS-Go|Player")
	FGoOnRegisterPlayerComplete GoOnRegisterPlayerComplete;

	void GoUnregisterPlayer(const APlayerState* NewPlayerState);
	UPROPERTY(BlueprintAssignable, Category="EOS-Go|Player")
	FGoOnUnregisterPlayerComplete GoOnUnregisterPlayerComplete;

protected:
	//~ To handle Registered Players functionality.
	void OnRegisteredPlayerComplete(FName SessionName, const TArray<FUniqueNetIdRef>& PlayerId, bool bWasSuccess);
	void OnUnregisteredPlayerComplete(FName SessionName, const TArray<FUniqueNetIdRef>& PlayerId, bool bWasSuccess);
	
private:
	//The subsystem designed to handle online functionality.
	IOnlineSessionPtr SessionInterface;
	IOnlineIdentityPtr Identity;
	
	//~ Delegates to add to the Online Session Interface delegate list. Each one has its own handle.
	FOnRegisterPlayersCompleteDelegate RegisterPlayersCompleteDelegate;
	FDelegateHandle RegisterPlayersCompleteDelegateHandle;
	FOnUnregisterPlayersCompleteDelegate UnregisterPlayersCompleteDelegate;
	FDelegateHandle UnregisterPlayersCompleteDelegateHandle;

};
