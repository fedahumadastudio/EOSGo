// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GoOverlay.h"
#include "Subsystem/GoSubsystem.h"
#include "Components/Button.h"
#include "Game/GoGameModeBase.h"

bool UGoOverlay::Initialize()
{
	if (!Super::Initialize()) return false;

	if (ExitSession_Button)
	{
		ExitSession_Button->OnClicked.AddDynamic(this, &UGoOverlay::ExitSessionButtonClicked);
	}
	/*
	if (FriendsList_Button)
	{
		FriendsList_Button->OnClicked.AddDynamic(this, &UGoOverlay::FriendsListButtonClicked);
	}
	*/
	return true;
}

void UGoOverlay::NativeDestruct()
{
	Super::NativeDestruct();
}

void UGoOverlay::GoOverlaySetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		GoSubsystem = GameInstance->GetSubsystem<UGoSubsystem>();
	}

	if (IsValid(GoSubsystem))
	{
		//~ Bind session callbacks.
		GoSubsystem->GoOnDestroySessionComplete.AddDynamic(this, &UGoOverlay::OnDestroySession);
		GoSubsystem->GoOnStartSessionComplete.AddDynamic(this, &UGoOverlay::OnStartSession);
		//GoSubsystem->GoOnReadFriendsListComplete.AddUObject(this, &UGoOverlay::OnReadFriendsList);
	}
}


void UGoOverlay::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		LogMessage("Failed destroying session!");
		ExitSession_Button->SetIsEnabled(true);
		return;
	}
	
	if (UWorld* World = GetWorld())
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode())
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
	LogMessage("Session destroyed successfully!");
}

void UGoOverlay::OnStartSession(bool bWasSuccessful)
{
}

/*
void UGoOverlay::OnReadFriendsList(TArray<TSharedRef<FOnlineFriend>> FriendsArray, bool bWasSuccessful)
{
	if (!IsValid(GoSubsystem)) return;
	if (FriendsArray.IsEmpty()) LogMessage("Friends list empty!");

	for (TSharedRef<FOnlineFriend> Friend : FriendsArray)
	{
		FString FriendName = Friend.Get().GetRealName();
		LogMessage(FriendName);
	}
}

void UGoOverlay::FriendsListButtonClicked()
{
	if (GoSubsystem) GoSubsystem->GoReadFriendsList();
}
*/

void UGoOverlay::ExitSessionButtonClicked()
{
	ExitSession_Button->SetIsEnabled(false);
	if (GoSubsystem) GoSubsystem->GoDestroySession();
}

void UGoOverlay::MenuTearDown()
{
	RemoveFromParent();
	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			const FInputModeGameOnly InputData;
			PlayerController->SetInputMode(InputData);
		}
	}
}
