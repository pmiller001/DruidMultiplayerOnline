// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),

///Below is the problem line of Code
DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))

{
	IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get();
	if(SubSystem)
	{
		SessionInterface = SubSystem->GetSessionInterface();
		
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!SessionInterface.IsValid())
		{
			return;
		}
auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
		{
			SessionInterface->DestroySession(NAME_GameSession);
		}
	//Store the delegate in a FDelegatehandle so we can later remove it from the delegate list.
	CreateSessionCompleteDelegateHandle = 	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "Null" ? true : false;
LastSessionSettings->NumPublicConnections = NumPublicConnections;
LastSessionSettings->bAllowJoinInProgress = true;
LastSessionSettings->bAllowJoinViaPresence = true;
LastSessionSettings->bShouldAdvertise = true;
LastSessionSettings->bUsesPresence = true;
LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if(!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		//Broadcast our own customDelegate
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}

	
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResult)
{
	if(!SessionInterface.IsValid())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("OnlineSessionInterface Is not Valid"));
		}
		return;
	}
	FindSessionCompleteDelegateHandle =  SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResult;
	LastSessionSearch->bIsLanQuery  = IOnlineSubsystem::Get()->GetSubsystemName() == "Null" ? true : false;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if(SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(),LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
		MultiplayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
	
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if(!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("SessionInterface Is not Valid"));
		}
		return;
	}
JoinSessionCompleteDelegateHandle = 	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//Line commented out on 8/1/2024 to test if this was the issue. 
	//SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(),LastSessionSearch.ToSharedRef());

	if(!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		LastSessionSettings->bUseLobbiesIfAvailable = true;
	}
MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
	
}


void UMultiplayerSessionsSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
	}
	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		MultiplayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	MultiplayerOnFindSessionComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
