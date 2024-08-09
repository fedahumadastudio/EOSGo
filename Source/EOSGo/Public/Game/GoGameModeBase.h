// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GoGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class EOSGO_API AGoGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGoGameModeBase();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UPROPERTY(EditDefaultsOnly, Category="EOS-Go")
	bool bDisplayScreenMessage = true;

private:
	IOnlineSessionPtr SessionInterface;
	
	bool RegisterPlayer(APlayerController* NewPlayer) const;
};
