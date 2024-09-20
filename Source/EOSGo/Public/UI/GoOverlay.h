// Copyright (c) 2024 Fedahumada Studio. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GoOverlay.generated.h"
class UTextBlock;
class AGoGameModeBase;
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
	
private:
	//The subsystem designed to handle online functionality.
	TObjectPtr<UGoSubsystem> GoSubsystem;
	
	UPROPERTY(meta = (BindWidget))
	UButton* ExitSession_Button;
	UFUNCTION()
	void ExitSessionButtonClicked();

	UPROPERTY(meta = (BindWidget))
	UButton* StartSession_Button;
	UFUNCTION()
	void StartSessionButtonClicked();
	
	void MenuTearDown();
};
