// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "GameFramework/GameStateBase.h"
#include "GoGameStateBase.generated.h"
class AGoGameModeBase;


//~ DELEGATES
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
	void OnRegisterPlayer(bool bWasSuccessful);

private:
	TObjectPtr<AGoGameModeBase> GoGameModeBase;
	
	UPROPERTY(ReplicatedUsing="OnRep_PlayerList")
	TArray<FName> PlayerList;
	
	UFUNCTION()
	void OnRep_PlayerList();
	
};
