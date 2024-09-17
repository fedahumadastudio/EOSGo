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

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRegisteredPlayerListChanged(FName SessionName, bool bWasSuccessful);

private:
	TObjectPtr<AGoGameModeBase> GoGameModeBase;
	TObjectPtr<UGoSubsystem> GoSubsystem;
	IOnlineSessionPtr SessionInterface;
	
	UPROPERTY(ReplicatedUsing="OnRep_PlayerList")
	TArray<FName> PlayerList;
	UFUNCTION()
	void OnRep_PlayerList();

	void CheckSessionFull(FName SessionName);
	void UpdateSessionAdvertising(bool InShouldAdvertise);
};
