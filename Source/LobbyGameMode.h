// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiCasualGameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MULTICASUAL_API ALobbyGameMode : public AMultiCasualGameMode
{
	GENERATED_BODY()
public:
	ALobbyGameMode();

	void PostLogin(APlayerController* NewPlayer) override;
	void Logout(AController* Exiting) override;
	void StartGame();

protected:
	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;

private:

	uint32 NumberOfPlayers = 0;

	FTimerHandle GameStartTimer;

	UPROPERTY()
	class ALobbyHUD* LobbyHUD;

public:
	FORCEINLINE uint32 GetNumberOfPlayers() { return NumberOfPlayers; }
};
