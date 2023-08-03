// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTICASUAL_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UOverheadWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	void SetDisplayText(FString TextToDisplay);

	UFUNCTION(BlueprintCallable)
	void SetDisplayTextWithTimer(FString Text);

	void SetCurrentMessage(const FString& Text) { CurrentMessage = Text; }
	const FString& GetCurrentMessage() { return CurrentMessage; }

protected:
	void NativeDestruct() override;

private:

	FTimerHandle TextEraseTimer;

	FTimerDelegate TextEraseDelegate;

	UPROPERTY(EditAnywhere)
	float TextEraseTime = 2.f;

	FString CurrentMessage;
};
