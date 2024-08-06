// © 2024 fedahumada. Almost all rights reserved. Violators will be forced to debug code written in Comic Sans.


#include "Game/GoGameModeBase.h"
#include "OnlineSubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

AGoGameModeBase::AGoGameModeBase()
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
}

void AGoGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!RegisterPlayer(NewPlayer)) return;

	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	FString PlayerUsername = PlayerState->GetPlayerName();
	int32 PlayerId = PlayerState->GetPlayerId();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(999, 999.f, FColor::Blue, TEXT("Players in game:"));
		GEngine->AddOnScreenDebugMessage(PlayerId, 999.f, FColor::Green, FString::Printf(TEXT("%s - Connected"), *PlayerUsername));
	}
}

void AGoGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>();
	FString PlayerUsername = PlayerState->GetPlayerName();
	int32 PlayerId = PlayerState->GetPlayerId();
	if (GEngine) GEngine->AddOnScreenDebugMessage(PlayerId, 999.f, FColor::Red, FString::Printf(TEXT("%s - Disconnected"), *PlayerUsername));
}

bool AGoGameModeBase::RegisterPlayer(APlayerController* NewPlayer) const
{
	if(NewPlayer == nullptr) return false;

	FUniqueNetIdRepl UniqueNetIdRepl;
	if(NewPlayer->IsLocalController())
	{
		if(const ULocalPlayer* LocalPlayer = NewPlayer->GetLocalPlayer())
		{
			UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			const UNetConnection* RemoteNetConnection = Cast<UNetConnection>(NewPlayer->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}
	}
	else
	{
		const UNetConnection* RemoteNetConnection = Cast<UNetConnection>(NewPlayer->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;
	}

	const TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	check(UniqueNetId != nullptr);

	if(SessionInterface->RegisterPlayer(NAME_GameSession, *UniqueNetId, false))
	{
		UE_LOG(LogTemp, Warning, TEXT("Registration Successful"));
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Registration Failed"));
		return false;
	}
}
