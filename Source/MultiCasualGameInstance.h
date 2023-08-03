// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MenuSystem/MenuInterface.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "HideGamePlayerInfo.h"
#include "MultiCasualGameInstance.generated.h"



/**
 * 
 */
UCLASS()
class MULTICASUAL_API UMultiCasualGameInstance : public UGameInstance, public IMenuInterface
{
	GENERATED_BODY()
public:
	UMultiCasualGameInstance(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void LoadMenuWidget();

	/*UFUNCTION()
	void InGameLoadMenu();*/

	UFUNCTION(Exec)
	virtual void Host(FString ServerName) override;

	UFUNCTION(Exec)
	virtual void Join(uint32 Index) override;

	virtual void LoadMainMenu() override;

	virtual void RefreshServerList() override;

	void StartSession();


private:
	UPROPERTY()
	TSubclassOf<class UUserWidget> MenuClass;

	//TSubclassOf<class UUserWidget> InGameMenuClass;

	UPROPERTY()
	class UMainMenu* Menu;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	FString DesiredServerName;
	void CreateSession();

	//**
	// Game Management
	//**

	uint32 PlayersCount = 0;

	UPROPERTY()
	TArray<FPlayerInfo> PlayersInfo;

	int32 CurrentStageIndex = 1;
	int32 MaxStageIndex = 2;

	bool bKeyInfoVisible = true;

public:
	void SetPlayersCount(uint32 Players) { PlayersCount = Players; }
	uint32 GetPlayersCount() { return PlayersCount; }

	void UpdatePlayerInfo(const FPlayerInfo& InPlayerInfo);
	float GetPlayerScore(const FString& InPlayerName);
	TArray<FPlayerInfo> GetPlayersInfo() { return PlayersInfo; }

	void SetMaxStageIndex(int32 MaxIndex) { MaxStageIndex = MaxIndex; }
	void AddStageIndex() { CurrentStageIndex++; }
	int32 GetCurrentStageIndex() { return CurrentStageIndex; }
	bool IsLastStage() { return CurrentStageIndex >= MaxStageIndex; }

	// Show Key Info
	void SetKeyInfoVisible(bool bIsVisible) { bKeyInfoVisible = bIsVisible;}
	bool GetKeyInfoVisible() { return bKeyInfoVisible; }
};
