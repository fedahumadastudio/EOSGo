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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:	
	UFUNCTION(BlueprintCallable, Category="EOS-Go|Setup")
	void GoMenuSetup(FString LobbyMapPath);

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

	//~ Menu setup - session creation parameters
	FString LobbyMap {FString(TEXT("/EOSGo/Maps/LobbyMap?listen"))};
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	int32 ServerPrivateJoinId{0};
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	FString MatchType {FString(TEXT("DUO"))};
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	int32 NumberOfConnections{2};
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	bool bIsPrivate{false};
	
	UPROPERTY(meta = (BindWidget))
	UButton* HostLobby_Button;
	UPROPERTY(meta = (BindWidget))
	UButton* JoinLobby_Button;
	UPROPERTY(meta = (BindWidget))
	UButton* Login_Button;
	UPROPERTY(meta = (BindWidget))
	UButton* Quit_Button;

	UFUNCTION()
	void LoginButtonClicked();
	UFUNCTION()
	void HostLobbyButtonClicked();
	UFUNCTION()
	void JoinLobbyButtonClicked();
	UFUNCTION()
	void QuitButtonClicked();
	
	void MenuTearDown();
};
