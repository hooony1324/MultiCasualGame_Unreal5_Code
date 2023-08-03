// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "UObject/NoExportTypes.h"
#include "Templates/SharedPointer.h"
#include "HideGameState.generated.h"

/**
 * 
 */
UCLASS()
class MULTICASUAL_API AHideGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AddSpectatedPlayer(class AMultiCasualGameCharacter* Character);
	void EmptySpectatedPlayers();

	void UpdateTopScore(class AHideGamePlayerState* ScoringPlayer);

	TArray<class AMultiCasualGameCharacter*> GetSpectatedPlayers();
	const TArray<class AHideGamePlayerState*>& GetTopScoringPlayers();
	
	void AddOrUpdatePlayerState(AHideGamePlayerState* PlayerState);


private:
	UPROPERTY(Replicated)
	TArray<class AHideGamePlayerState*> TopScoringPlayers;

	float TopScore = 0.f;

	UPROPERTY(Replicated)
	TObjectPtr<class AHideGamePlayerState> Seeker;

	UPROPERTY(Replicated)
	TArray<class AMultiCasualGameCharacter*> SpectatedPlayers;


public:
	void SetSeeker(AHideGamePlayerState* InPlayerState) { Seeker = InPlayerState; }
	AHideGamePlayerState* GetSeeker() { return Seeker; }
};
