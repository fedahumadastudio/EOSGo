// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GoSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class EOSGO_API UGoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UGoSubsystem();

	//~ To handle session functionality. The widget blueprint will call these.
	void GoCreateSession(int32 NumberOfPublicConnections, FName MatchType);

private:
	IOnlineSessionPtr SessionInterface;
};
