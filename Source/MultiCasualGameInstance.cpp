// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiCasualGameInstance.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuWidget.h"
#include "UObject/UObjectGlobals.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"
#include "Net/UnrealNetwork.h"

const static FName SESSION_NAME = TEXT("Game");
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");

UMultiCasualGameInstance::UMultiCasualGameInstance(const FObjectInitializer& ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/Blueprints/Widget/WBP_MainMenu"));		
	if (!ensure(MenuBPClass.Class != nullptr)) return;
	MenuClass = MenuBPClass.Class;

}

void UMultiCasualGameInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(UMultiCasualGameInstance, PlayersInfo);
}

void UMultiCasualGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				0,
				2,
				FColor::Green,
				FString::Printf(TEXT("Found Subsystem %s"), *OnlineSubsystem->GetSubsystemName().ToString()));
		}

		SessionInterface = OnlineSubsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiCasualGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMultiCasualGameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiCasualGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiCasualGameInstance::OnJoinSessionComplete);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Found No Subsystem"));
		}
	}

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UMultiCasualGameInstance::OnNetworkFailure);
	}

}

void UMultiCasualGameInstance::CreateSession()
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;

		FName SubSystemName = IOnlineSubsystem::Get()->GetSubsystemName();
		GEngine->AddOnScreenDebugMessage(
			0,
			10,
			FColor::Green,
			FString::Printf(TEXT("CreateSession, Found Subsystem : %s "), *SubSystemName.ToString())
		);

		SessionSettings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
		SessionSettings.NumPublicConnections = 5;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		SessionSettings.bUseLobbiesIfAvailable = true;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bAllowJoinViaPresence = true;
		SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.BuildUniqueId = 1;

		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);

		if (Menu)
		{
			Menu->GetHostButton()->SetIsEnabled(false);
		}
	}
}

void UMultiCasualGameInstance::Host(FString ServerName)
{
	DesiredServerName = ServerName;


	if (SessionInterface.IsValid())
	{
		auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (ExistingSession != nullptr)
		{
			SessionInterface->DestroySession(SESSION_NAME);
		}
		else
		{
			CreateSession();
		}
	}
}

void UMultiCasualGameInstance::Join(uint32 Index)
{
	if (!SessionInterface.IsValid()) return;
	if (!SessionSearch.IsValid()) return;

	if (Menu != nullptr)
	{
		Menu->Teardown();
	}

	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
}

void UMultiCasualGameInstance::StartSession()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->StartSession(SESSION_NAME);
	}
}

void UMultiCasualGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
	UEngine* Engine = GetEngine();

	if (!Success)
	{
		if (Engine)
		{
			Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Failed to Create Session"));

			if (Menu)
			{
				Menu->GetHostButton()->SetIsEnabled(true);
			}
		}
		return;
	}

	if (Menu != nullptr)
	{
		Menu->Teardown();
	}

	if (Engine)
	{
		Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting..."));
	}

	UWorld* World = GetWorld();
	if (World)
	{
		World->ServerTravel("/Game/Maps/Lobby?listen");
	}

	PlayersCount = 1;
}

void UMultiCasualGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
	if (Success)
	{
		CreateSession();
	}
}

void UMultiCasualGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	if (SessionSearch.IsValid())
	{
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		UE_LOG(LogTemp, Warning, TEXT("Start Find Session"));
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UMultiCasualGameInstance::OnFindSessionsComplete(bool Success)
{
	if (Success && SessionSearch.IsValid() && Menu != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			10,
			FColor::Green,
			FString::Printf(TEXT("Finished Find Session %d Rooms "), SessionSearch->SearchResults.Num())
		);

		UE_LOG(LogTemp, Warning, TEXT("Finished Find Session"));

		TArray<FServerData> ServerDatas;
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found session names: %s"), *SearchResult.GetSessionIdStr());
			FServerData ServerData;
			ServerData.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
			ServerData.CurrentPlayers = ServerData.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
			ServerData.HostUsername = SearchResult.Session.OwningUserName;
			FString ServerName;
			if (SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
			{
				ServerData.Name = ServerName;
			}
			else
			{
				ServerData.Name = "Failed to find Server Name";
			}
			ServerDatas.Add(ServerData);


			GEngine->AddOnScreenDebugMessage(
				0,
				10,
				FColor::Green,
				FString::Printf(TEXT("Server Name find : %s"), *ServerData.Name)
			);
		}

		Menu->SetServerList(ServerDatas);
	}
}

void UMultiCasualGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid()) return;

	FString Address;
	if (!SessionInterface->GetResolvedConnectString(SessionName, Address))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				0,
				5,
				FColor::Red,
				FString(TEXT("OnJoinSession ConnectString Failed"))
			);
		}
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
	}

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (PlayerController)
	{
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}

	PlayersCount++;
}

void UMultiCasualGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	LoadMainMenu();

	PlayersCount--;
}

void UMultiCasualGameInstance::LoadMainMenu()
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (PlayerController)
	{
		PlayerController->ClientTravel("/Game/Maps/MainMenu", ETravelType::TRAVEL_Absolute);
	}
}

void UMultiCasualGameInstance::LoadMenuWidget()
{
	if (!ensure(MenuClass != nullptr)) return;

	Menu = CreateWidget<UMainMenu>(this, MenuClass);

	if (!ensure(Menu != nullptr)) return;

	Menu->Setup();

	Menu->SetMenuInterface(this);
}


void UMultiCasualGameInstance::UpdatePlayerInfo(const FPlayerInfo& InPlayerInfo)
{
	FPlayerInfo* FoundInfo = PlayersInfo.FindByPredicate([=](const FPlayerInfo& Info)
		{
			return Info.PlayerName == InPlayerInfo.PlayerName;
		});

	if (FoundInfo)
	{
		FoundInfo->PlayerScore += InPlayerInfo.PlayerScore;
	}
	else
	{
		PlayersInfo.AddUnique(InPlayerInfo);
	}

}

float UMultiCasualGameInstance::GetPlayerScore(const FString& InPlayerName)
{
	FPlayerInfo* FoundInfo = PlayersInfo.FindByPredicate([=](const FPlayerInfo& Info)
		{
			return Info.PlayerName == InPlayerName;
		});


	if (FoundInfo)
	{
		return FoundInfo->PlayerScore;
	}
	else
	{
		return 0.f;
	}
}


