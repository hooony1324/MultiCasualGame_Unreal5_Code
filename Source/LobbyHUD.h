// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LobbyHUD.generated.h"

/**
 * 
 */
UCLASS()
class MULTICASUAL_API ALobbyHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable)
	void StartButtonClicked();


protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	// Lobby HUD
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> LobbyWidgetClass; // (¼­¹öOnly)StartButtonÀ§Á¬

	UPROPERTY()
	UUserWidget* LobbyWidget;

	bool bStartButtonClicked = false;

public:
	UUserWidget* GetLobbyWidget() { return LobbyWidget; }
	FORCEINLINE bool IsStartButtonClicked() { return bStartButtonClicked; }
};