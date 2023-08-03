// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HideGameWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTICASUAL_API UHideGameWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintImplementableEvent)
	void EnableBlindfold(bool bEnable);

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GameTime;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthAmountText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreText;

	// for debug
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MatchStateText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerStateText;

protected:

private:

};
