// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HideGamePlayerInfo.h"
#include "MatchAnnouncement.generated.h"

/**
 * 
 */
UCLASS()
class MULTICASUAL_API UMatchAnnouncement : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SeekerSelecting();
	void HideStart(const FString& SeekerName);
	void AnnounceMessage(const FString& Message);
	void ShowScores(const TArray<FPlayerInfo>& PlayerStates);
	void ShowScores_MatchEnd(const TArray<class AHideGamePlayerState*>& PlayerStates);

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncementText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoresText;	

protected:
	bool Initialize() override;


private:

	FTimerDelegate HideAnnouncementDelegate;
	FTimerHandle HideAnnouncementTimer;
	float DelayTime = 2.5f;

};
