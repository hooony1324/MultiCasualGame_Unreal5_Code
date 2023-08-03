// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/Actor.h"

UOverheadWidget::UOverheadWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	// erase displaytext
	TextEraseDelegate = FTimerDelegate::CreateUObject(
		this,
		&UOverheadWidget::SetDisplayText,
		FString()
	);
}

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::SetDisplayTextWithTimer(FString Text)
{
	SetDisplayText(Text);

	GetWorld()->GetTimerManager().SetTimer(
		TextEraseTimer,
		TextEraseDelegate,
		TextEraseTime,
		false
	);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}

