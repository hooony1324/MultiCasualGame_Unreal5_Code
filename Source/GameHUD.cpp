// Fill out your copyright notice in the Description page of Project Settings.


#include "GameHUD.h"

#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Chat.h"
#include "HideGameWidget.h"
#include "MatchAnnouncement.h"
#include "Components/TextBlock.h"

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();

	ensure(BasicCrosshairsCenter);
}


void AGameHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		else
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(BasicCrosshairsCenter, ViewportCenter, Spread, FLinearColor::White);
		}

		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}
}

void AGameHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
	);
}

void AGameHUD::AddHideGameWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && HideGameWidgetClass)
	{
		HideGameWidget = CreateWidget<UHideGameWidget>(PlayerController, HideGameWidgetClass);
		HideGameWidget->AddToViewport();
	}
}

void AGameHUD::AddMatchAnnouncementWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && MatchAnnouncementClass)
	{
		MatchAnnouncement = CreateWidget<UMatchAnnouncement>(PlayerController, MatchAnnouncementClass);
		MatchAnnouncement->AddToViewport();
	}
}

void AGameHUD::ClearAllWidgets()
{
	HideGameWidget->RemoveFromParent();
	MatchAnnouncement->RemoveFromParent();
}
