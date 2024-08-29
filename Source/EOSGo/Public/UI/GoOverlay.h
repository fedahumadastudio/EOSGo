// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GoOverlay.generated.h"
class FOnlineFriend;
class UGoSubsystem;
class UButton;

/**
 * 
 */
UCLASS()
class EOSGO_API UGoOverlay : public UUserWidget
{
	GENERATED_BODY()

protected: //virtual
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;
	
public:
	UFUNCTION(BlueprintCallable, Category="EOS-Go|Setup")
	void GoOverlaySetup();

protected:
	//~ Session callbacks for the custom delegates on the GoSubsystem.
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	
	//void OnReadFriendsList(TArray<TSharedRef<FOnlineFriend>> FriendsArray, bool bWasSuccessful);
	
private:
	//The subsystem designed to handle online functionality.
	TObjectPtr<UGoSubsystem> GoSubsystem;

	//~ Overlay setup - session creation parameters
	FName MainMenuMap {FString(TEXT("/EOSGo/Maps/MainMenuMap"))};
	
	UPROPERTY(meta = (BindWidget))
	UButton* ExitSession_Button;

	UFUNCTION()
	void ExitSessionButtonClicked();
	
	void MenuTearDown();
};
