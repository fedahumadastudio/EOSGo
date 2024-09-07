// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.

#include "Game/GoGameStateBase.h"
#include "Game/GoGameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AGoGameStateBase::AGoGameStateBase()
{
	SetReplicates(true);

	if (AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(GetWorld()))
	{
		GoGameModeBase = Cast<AGoGameModeBase>(GameModeBase);
	}
	
	if (GoGameModeBase)
	{
		//~ Bind callbacks.
		GoGameModeBase->GoOnRegisterPlayerComplete.AddDynamic(this, &AGoGameStateBase::OnRegisteredPlayerListChanged);
		GoGameModeBase->GoOnUnregisterPlayerComplete.AddDynamic(this, &AGoGameStateBase::OnRegisteredPlayerListChanged);
	}
}

void AGoGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Add the PlayerList to the list of replicated properties.
	DOREPLIFETIME(AGoGameStateBase, PlayerList);
}

void AGoGameStateBase::OnRegisteredPlayerListChanged(bool bWasSuccessful)
{
	if (!bWasSuccessful) return;
	
	//~ UPDATE PLAYER LIST
	if (HasAuthority())
	{
		PlayerList.Empty();
		for (TObjectPtr<APlayerState> PlayerState : PlayerArray)
		{
			PlayerList.AddUnique(FName(PlayerState->GetPlayerName()));
		}
		//~ Broadcast the updated player list.
		OnPlayerListChanged.Broadcast(PlayerList);
	}
}

void AGoGameStateBase::OnRep_PlayerList()
{
	OnPlayerListChanged.Broadcast(PlayerList);
}
