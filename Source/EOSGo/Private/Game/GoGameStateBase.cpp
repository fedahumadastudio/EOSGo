// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.

#include "Game/GoGameStateBase.h"
#include "EOSGo.h"
#include "OnlineSessionSettings.h"
#include "Game/GoGameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Subsystem/GoSubsystem.h"

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
		GoGameModeBase->GoOnRegisterPlayerComplete.AddDynamic(this, &AGoGameStateBase::OnRegisteredPlayer);
		GoGameModeBase->GoOnUnregisterPlayerComplete.AddDynamic(this, &AGoGameStateBase::OnUnregisteredPlayer);
	}
}

void AGoGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Add the PlayerList to the list of replicated properties.
	DOREPLIFETIME(AGoGameStateBase, PlayerList);
}


void AGoGameStateBase::OnRegisteredPlayer(bool bWasSuccessful)
{
	if (!bWasSuccessful) return;
	if (!HasAuthority()) return;
	CheckSessionToAdvertise(true);
	PlayerListChanged();
}
void AGoGameStateBase::OnUnregisteredPlayer(bool bWasSuccessful)
{
	if (!bWasSuccessful) return;
	if (!HasAuthority()) return;
	CheckSessionToAdvertise(false);
	PlayerListChanged();
}


void AGoGameStateBase::CheckSessionToAdvertise(bool bIsRegisteringPlayer)
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
	
	if (!SessionInterface.IsValid()) return;

	FOnlineSession* GoSession = SessionInterface->GetNamedSession(NAME_GameSession);
	FString Value;
	GoSession->SessionSettings.Get(FName("MATCH_TYPE"), Value);

	if (Value.IsEmpty()) LogMessage("Retrieved MatchType variable via online service is empty");
	FName Type = FName(Value);

	if (bIsRegisteringPlayer)
	{
		if (Type == "DUO")
		{
			LogMessage("Updating DUO session");
			if (PlayerArray.Num() == 2) UpdateSessionAdvertising(false);
		}
		else if (Type == "TRIO")
		{
			LogMessage("Updating TRIO session");
			if (PlayerArray.Num() == 3) UpdateSessionAdvertising(false);
		}
		else if (Type == "SQUAD")
		{
			LogMessage("Updating SQUAD session");
			if (PlayerArray.Num() == 4) UpdateSessionAdvertising(false);
		}
	}
	else
	{
		if (Type == "DUO")
		{
			LogMessage("Updating DUO session");
			if (PlayerArray.Num() < 2) UpdateSessionAdvertising(true);
		}
		else if (Type == "TRIO")
		{
			LogMessage("Updating TRIO session");
			if (PlayerArray.Num() < 3) UpdateSessionAdvertising(true);
		}
		else if (Type == "SQUAD")
		{
			LogMessage("Updating SQUAD session");
			if (PlayerArray.Num() < 4) UpdateSessionAdvertising(true);
		}
	}
}

void AGoGameStateBase::UpdateSessionAdvertising(bool InShouldAdvertise)
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		GoSubsystem = GameInstance->GetSubsystem<UGoSubsystem>();
	}

	//~ Set new session settings.
	TSharedPtr<FOnlineSessionSettings> NewSessionSettings = MakeShared<FOnlineSessionSettings>();
	NewSessionSettings->bShouldAdvertise = InShouldAdvertise;

	//~ Call update session
	if (GoSubsystem) GoSubsystem->UpdateSession(*NewSessionSettings);
}


void AGoGameStateBase::PlayerListChanged()
{
	//~ UPDATE PLAYER LIST
	PlayerList.Empty();
	for (TObjectPtr<APlayerState> PlayerState : PlayerArray)
	{
		PlayerList.AddUnique(FName(PlayerState->GetPlayerName()));
	}
	//~ Broadcast the updated player list.
	OnPlayerListChanged.Broadcast(PlayerList);
}
void AGoGameStateBase::OnRep_PlayerList()
{
	OnPlayerListChanged.Broadcast(PlayerList);
}
