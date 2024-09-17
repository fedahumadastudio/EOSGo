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

void AGoGameStateBase::OnRegisteredPlayerListChanged(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful) return;
	if (!HasAuthority()) return;
	CheckSessionFull(SessionName);
	
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

void AGoGameStateBase::CheckSessionFull(FName SessionName)
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
	
	if (!SessionInterface.IsValid()) return;

	FOnlineSession* GoSession = SessionInterface->GetNamedSession(SessionName);
	FString Type;
	GoSession->SessionSettings.Get(FName("MATCH_TYPE"), Type);
	if (Type.IsEmpty()) LogMessage("Retrieved MatchType variable via online service is empty");
	
	if (Type == "DUO")
	{
		if (PlayerArray.Num() == 2) UpdateSessionAdvertising(false);
	}
	else if (Type == "TRIO")
	{
		if (PlayerArray.Num() == 3) UpdateSessionAdvertising(false);
	}
	else if (Type == "SQUAD")
	{
		if (PlayerArray.Num() == 4) UpdateSessionAdvertising(false);
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
