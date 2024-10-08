// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.

#include "Game/GoGameModeBase.h"
#include "OnlineSubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Engine.h"
#include "Engine/NetConnection.h"
#include "Engine/LocalPlayer.h"
#include "Game/GoGameStateBase.h"
#include "EOSGo.h"


AGoGameModeBase::AGoGameModeBase() :
/*Bind Delegates*/
RegisterPlayersCompleteDelegate(FOnRegisterPlayersCompleteDelegate::CreateUObject(this, &ThisClass::OnRegisteredPlayerComplete)),
UnregisterPlayersCompleteDelegate(FOnUnregisterPlayersCompleteDelegate::CreateUObject(this, &ThisClass::OnUnregisteredPlayerComplete))
{
	GameStateClass = AGoGameStateBase::StaticClass();
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
		Identity = Subsystem->GetIdentityInterface();
	}
}

void AGoGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (APlayerState* NewPlayerState = NewPlayer->GetPlayerState<APlayerState>())
	{
		GoRegisterPlayer(NewPlayerState);
	}
}
void AGoGameModeBase::Logout(AController* ExitingPlayer)
{
	Super::Logout(ExitingPlayer);

	if (APlayerState* ExitingPlayerState = Cast<APlayerState>(ExitingPlayer->PlayerState))
	{
		GoUnregisterPlayer(ExitingPlayerState);
	}
}


void AGoGameModeBase::OnRegisteredPlayerComplete(FName SessionName, const TArray<FUniqueNetIdRef>& PlayerId, bool bWasSuccess)
{
	//~ If registration was successful, clear delegate of the delegate list.
	if (SessionInterface) SessionInterface->ClearOnRegisterPlayersCompleteDelegate_Handle(RegisterPlayersCompleteDelegateHandle);

	//~ Broadcast Go GameModeBase Delegate - Registration successful.
	LogMessage("Registration Successful");
	GoOnRegisterPlayerComplete.Broadcast(true);
	
}
void AGoGameModeBase::GoRegisterPlayer(const APlayerState* NewPlayerState)
{
	if (!SessionInterface.IsValid()) return;
	
	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	RegisterPlayersCompleteDelegateHandle = SessionInterface->AddOnRegisterPlayersCompleteDelegate_Handle(RegisterPlayersCompleteDelegate);
	
	//~ REGISTER PLAYER
	if(!SessionInterface->RegisterPlayer(NAME_GameSession, *NewPlayerState->GetUniqueId(), false))
	{
		//~ If it doesn't register the player, clear delegate of the delegate list.
		SessionInterface->ClearOnRegisterPlayersCompleteDelegate_Handle(RegisterPlayersCompleteDelegateHandle);
		//~ Broadcast Go GameModeBase Delegate - Registration not successful.
		LogMessage("Registration Failed");
		GoOnRegisterPlayerComplete.Broadcast(false);
	}
}


void AGoGameModeBase::OnUnregisteredPlayerComplete(FName SessionName, const TArray<FUniqueNetIdRef>& PlayerId, bool bWasSuccess)
{
	//~ If unregistration was successful, clear delegate of the delegate list.
	if (SessionInterface) SessionInterface->ClearOnUnregisterPlayersCompleteDelegate_Handle(UnregisterPlayersCompleteDelegateHandle);

	//~ Broadcast Go GameModeBase Delegate - Unregistration successful.
	LogMessage("Unregistration Successful");	
	GoOnUnregisterPlayerComplete.Broadcast(true);
}
void AGoGameModeBase::GoUnregisterPlayer(const APlayerState* NewPlayerState)
{
	if (!SessionInterface.IsValid()) return;
	
	//~ Store the delegate in a FDelegateHandle, so we can later remove it from the delegate list.
	UnregisterPlayersCompleteDelegateHandle = SessionInterface->AddOnUnregisterPlayersCompleteDelegate_Handle(UnregisterPlayersCompleteDelegate);
	
	//~ UNREGISTER PLAYER
	if(!SessionInterface->UnregisterPlayer(NAME_GameSession, *NewPlayerState->GetUniqueId()))
	{
		//~ If it doesn't unregister the player, clear delegate of the delegate list.
		SessionInterface->ClearOnUnregisterPlayersCompleteDelegate_Handle(UnregisterPlayersCompleteDelegateHandle);
		//~ Broadcast Go GameModeBase Delegate - Unregistration not successful.
		LogMessage("Unregistration Failed");
		GoOnUnregisterPlayerComplete.Broadcast(false);
	}
}
