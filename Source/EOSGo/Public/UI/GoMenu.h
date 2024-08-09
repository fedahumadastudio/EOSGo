// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Blueprint/UserWidget.h"
#include "GoMenu.generated.h"
class UGoSubsystem;
class UButton;

/**
 * 
 */
UCLASS()
class EOSGO_API UGoMenu : public UUserWidget
{
	GENERATED_BODY()
	
protected: //virtual
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category="EOS-Go|Misc")
	void MenuSetup(int32 NumberOfPublicConnections = 3, FString TypeOfMatch = FString(TEXT("Lobby")), FString LevelPath = FString(TEXT("/EOSGo/Maps/LobbyMap")));

protected:
	//~ Session callbacks for the custom delegates on the GoSubsystem.
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool  bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	
private:
	TObjectPtr<UGoSubsystem> GoSubsystem; //The subsystem designed to handle online sessions' functionality.
	FString MapToTravel;
	FString MatchType;
	int32 NumberOfConnections;
	void MenuTearDown();
	
	UPROPERTY(meta = (BindWidget))
	UButton* Host_Button;
	UPROPERTY(meta = (BindWidget))
	UButton* Join_Button;
	UPROPERTY(meta = (BindWidget))
	UButton* Login_Button;

	UFUNCTION()
	void HostButtonClicked();
	UFUNCTION()
	void JoinButtonClicked();
	UFUNCTION()
	void LoginButtonClicked();
	
};
