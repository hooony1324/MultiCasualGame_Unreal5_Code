// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"
#include "MultiCasualGameInstance.h"
#include "LobbyHUD.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "MultiCasualPlayerController.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

ALobbyGameMode::ALobbyGameMode()
{
	HUDClass = ALobbyHUD::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

}

void ALobbyGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);	
	
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	++NumberOfPlayers;

}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	--NumberOfPlayers;
}

void ALobbyGameMode::StartGame()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	UMultiCasualGameInstance* GameInstance = Cast<UMultiCasualGameInstance>(GetGameInstance());

	if (World)
	{
		if (GameInstance)
		{
			// SteamSubsystem
			IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
			FName Name("NULL");
			int32 SteamSubsystemValid = !OnlineSubsystem->GetSubsystemName().Compare(Name);
			if (SteamSubsystemValid == 0)
			{
				int32 LoginPlayers = GameState.Get()->PlayerArray.Num();
				GameInstance->SetPlayersCount(LoginPlayers);

				bUseSeamlessTravel = true;
				World->ServerTravel(FString("/Game/Maps/HideGame?listen"));
			}
			// ¿¡µðÅÍ ListenServer
			else
			{
				//bUseSeamlessTravel = true;
				//World->ServerTravel("/Game/Maps/HideGame");
			}
		}
	}

}
