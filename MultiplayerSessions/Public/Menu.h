// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()


	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")));


private:
	///These variables are for the buttons
UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

UPROPERTY(meta = (BindWidget))
	 UButton* JoinButton;

UFUNCTION()
	void HostButtonClicked();

UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	int32 NumberOfPublicConnections{4};
	FString Matchtype{TEXT("FreeForAll")}; 

	///This is to access the subsystem designed to handle all online functionality
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;


	///
	///Callbacks for the custom Delegates on the multiplayer sessions subsystem
	///
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnStartSession(bool bWasSuccesful);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccesful);
	//virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld *InWorld) override;
};
