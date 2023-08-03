// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyHUD.h"
#include "Blueprint/UserWidget.h"
#include "LobbyGameMode.h"
#include "Chat.h"
#include "Kismet/GameplayStatics.h"

void ALobbyHUD::DrawHUD()
{
	Super::DrawHUD();
}

void ALobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	if (LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<UUserWidget>(GetWorld(), LobbyWidgetClass);
		LobbyWidget->AddToViewport();
	}
}

void ALobbyHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bStartButtonClicked)
	{
		LobbyWidget->RemoveFromParent();

		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (PlayerController)
		{
			ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
			if (LobbyGameMode)
			{
				LobbyGameMode->StartGame();
			}
		}
	}
}

void ALobbyHUD::StartButtonClicked()
{
	bStartButtonClicked = true;
}
