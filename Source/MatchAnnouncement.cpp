// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchAnnouncement.h"
#include "Components/TextBlock.h"
#include "GameFramework/Actor.h"
#include "HideGamePlayerState.h"


bool UMatchAnnouncement::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	HideAnnouncementDelegate.BindLambda([&] {
		this->SetVisibility(ESlateVisibility::Hidden);
		});


	return true;
}

void UMatchAnnouncement::SeekerSelecting()
{
	this->SetVisibility(ESlateVisibility::Visible);
	FString Announcement = TEXT("술래를 정하는 중입니다");
	AnnounceMessage(Announcement);
}

void UMatchAnnouncement::HideStart(const FString& SeekerName)
{
	this->SetVisibility(ESlateVisibility::Visible);
	FString Announcement = FString::Printf(TEXT(" %s을 피해 숨으세요!"), *SeekerName);
	AnnouncementText->SetText(FText::FromString(Announcement));
	ScoresText->SetText(FText());

	GetWorld()->GetTimerManager().SetTimer(
		HideAnnouncementTimer,
		HideAnnouncementDelegate,
		DelayTime,
		false
	);
}

void UMatchAnnouncement::AnnounceMessage(const FString& Message)
{
	this->SetVisibility(ESlateVisibility::Visible);
	FString Announcement = FString::Printf(TEXT("%s"), *Message);
	AnnouncementText->SetText(FText::FromString(Announcement));
	ScoresText->SetText(FText());

	GetWorld()->GetTimerManager().SetTimer(
		HideAnnouncementTimer,
		HideAnnouncementDelegate,
		DelayTime,
		false
	);
}

void UMatchAnnouncement::ShowScores(const TArray<FPlayerInfo>& PlayerInfos)
{
	this->SetVisibility(ESlateVisibility::Visible);
	FString Announcement = TEXT("점수");
	AnnouncementText->SetText(FText::FromString(Announcement));


	FString Scores;
	for (FPlayerInfo PlayerInfo : PlayerInfos)
	{
		FString Score = FString::Printf(TEXT("%s : %d점\n"), *PlayerInfo.PlayerName, (int32)PlayerInfo.PlayerScore);
		Scores.Append(Score);
	
	}
	ScoresText->SetText(FText::FromString(Scores));
}

void UMatchAnnouncement::ShowScores_MatchEnd(const TArray<AHideGamePlayerState*>& PlayerStates)
{
	this->SetVisibility(ESlateVisibility::Visible);
	FString Announcement = TEXT("우승!");
	AnnouncementText->SetText(FText::FromString(Announcement));

	FString Scores;
	for (TObjectPtr<APlayerState> PlayerState : PlayerStates)
	{
		AHideGamePlayerState* PState = Cast<AHideGamePlayerState>(PlayerState);
		if (PState)
		{
			FString Score = FString::Printf(TEXT("%s!!!\n"), *PState->GetPlayerName());
			Scores.Append(Score);
		}
	}
	ScoresText->SetText(FText::FromString(Scores));
}
