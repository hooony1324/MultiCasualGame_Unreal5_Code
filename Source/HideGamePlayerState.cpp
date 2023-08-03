// Fill out your copyright notice in the Description page of Project Settings.


#include "HideGamePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "MultiCasualGameCharacter.h"
#include "CombatComponent.h"
#include "MultiCasualPlayerController.h"

void AHideGamePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AHideGamePlayerState, Team);
}

void AHideGamePlayerState::SetTeam(ETeam InTeam)
{
	Team = InTeam;
}

void AHideGamePlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AMultiCasualGameCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMultiCasualPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}

}

void AHideGamePlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AMultiCasualGameCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMultiCasualPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

