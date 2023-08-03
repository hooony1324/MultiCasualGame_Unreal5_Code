// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
	public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairsColor;
};

/**
 * 
 */
UCLASS()
class MULTICASUAL_API AGameHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;

	//**
	// Match HUD Setting
	//**
	void AddHideGameWidget(); 
	void AddMatchAnnouncementWidget();

	void ClearAllWidgets();


	UPROPERTY(BlueprintReadOnly)
	class UHideGameWidget* HideGameWidget;

	UPROPERTY()
	class UMatchAnnouncement* MatchAnnouncement;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> HideGameWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> MatchAnnouncementClass;

	//**
	//  Crosshairs
	//**

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* BasicCrosshairsCenter;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
