// Fill out your copyright notice in the Description page of Project Settings.


#include "HideGameState.h"
#include "HideGamePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "MultiCasualGameCharacter.h"

void AHideGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHideGameState, Seeker);
	DOREPLIFETIME(AHideGameState, SpectatedPlayers);
	DOREPLIFETIME(AHideGameState, TopScoringPlayers);
}

void AHideGameState::AddSpectatedPlayer(AMultiCasualGameCharacter* Character)
{
	SpectatedPlayers.AddUnique(Character);
}

void AHideGameState::EmptySpectatedPlayers()
{
	SpectatedPlayers.Empty();
}

void AHideGameState::UpdateTopScore(AHideGamePlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

TArray<class AMultiCasualGameCharacter*> AHideGameState::GetSpectatedPlayers()
{
	return SpectatedPlayers;
}

const TArray<AHideGamePlayerState*>& AHideGameState::GetTopScoringPlayers()
{
	return TopScoringPlayers;
}


