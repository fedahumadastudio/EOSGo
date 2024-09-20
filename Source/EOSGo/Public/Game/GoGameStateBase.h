// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GoGameStateBase.generated.h"
class UGoSubsystem;
class AGoGameModeBase;

//~ GO GAME STATE DELEGATES
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerListChangedSignature, const TArray<FName>&, PlayerList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionStartedSignature, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class EOSGO_API AGoGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGoGameStateBase();
	
	UPROPERTY(BlueprintAssignable, Category="EOS-Go|Player")
	FOnPlayerListChangedSignature OnPlayerListChanged;
	UPROPERTY(BlueprintAssignable, Category="EOS-Go|Session")
	FOnSessionStartedSignature OnSessionStarted;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRegisteredPlayer(bool bWasSuccessful);
	UFUNCTION()
	void OnUnregisteredPlayer(bool bWasSuccessful);
	UFUNCTION()
	void OnStartedSession(bool bWasSuccessful);

private:
	TObjectPtr<AGoGameModeBase> GoGameModeBase;
	TObjectPtr<UGoSubsystem> GoSubsystem;
	IOnlineSessionPtr SessionInterface;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing="OnRep_MatchStartedText", meta=(AllowPrivateAccess="true"))
	FName MatchStartedText {FName(" ")};
	UFUNCTION()
	void OnRep_MatchStartedText() const;

	UPROPERTY(ReplicatedUsing="OnRep_PlayerList")
	TArray<FName> PlayerList;
	UFUNCTION()
	void OnRep_PlayerList() const;

	void CheckSessionToAdvertise(bool bIsRegisteringPlayer);
	void UpdateSessionAdvertising(bool InShouldAdvertise);
	void PlayerListChanged();
};
