// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HideGameTeam.h"
#include "HideGamePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MULTICASUAL_API AHideGamePlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;


	//**
	// Scoring
	//**
	virtual void OnRep_Score() override;

	void AddToScore(float ScoreAmount);

private:
	UPROPERTY(Replicated)
	ETeam Team = ETeam::ET_NoTeam;

	UPROPERTY()
	class AMultiCasualGameCharacter* Character;

	UPROPERTY()
	class AMultiCasualPlayerController* Controller;

public:
	void SetTeam(ETeam InTeam);
	ETeam GetTeam() { return Team; }
};
