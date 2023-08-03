// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HideGameTeam.h"
#include "MultiCasualPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTICASUAL_API AMultiCasualPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	//**
	// Chat
	//**
	void FocusChat();
	void FocusGame();
	void SendMessage(const FText& Text);

	//**
	// GamePlay
	//**
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void OnMatchStateSet(FName State);
	void SetHUDTime();

	void HandleWarmUpStarted();
	void HandleInProgressStarted();
	void HandleHidingStarted();
	void HandleSeekingStarted();
	void HandleCooldownStarted();

	void SepectateNextPlayer();
	void SepectatePrevPlayer();

	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void ReceivedPlayer() override;

	// Init process after beginplay
	void PollInit();

private:

	UPROPERTY()
	class AMultiCasualGameCharacter* PlayerCharacter;

	//**
	// Chat
	// - Server RPC > Client RPC
	//**
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> ChatWidgetClass;

	UPROPERTY()
	class UChat* ChatWidget;

	UPROPERTY()
	class UUserWidget* LobbyWidget;

	UPROPERTY()
	class UOverheadWidget* OverheadWidget;

	bool bIsLobby = false;

	UFUNCTION(Server, Reliable)
	void ServerSendMessage(const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientSendMessage(const FString& Message);

	//**
	// GamePlayHUD
	// - Replicate MatchState
	//**
	UPROPERTY()
	class AGameHUD* GameHUD;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	//**
	// Sync Match Time
	// - Client Request (ServerRPC) > Server Respond (ClientRPC) > Check Server Time
	//**

	float GetServerTime();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float SingleTripTime = 0.f;
	float ClientServerDelta = 0.f;

	// 일정 주기마다 Sync
	void CheckTimeSync(float DeltaTime);
	float TimeSyncFrequency = 3.f;
	float TimeSyncRunningTime = 0.f;

	float LevelStartingTime = 0.f;
	float WarmupTime = 0.f;
	float ProgressTime = 0.f;
	float HidingTime = 0.f;
	float SeekingTime = 0.f;
	float CoolDownTime = 0.f;
	uint32 CountdownInt = 0;

	//**
	// 게임 입장 처리
	//**

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Progress, float Hiding, float Seeking, float Cooldown, float LevelStarting);

	void SetWarmupHud();

	void SetCombatMode(ETeam Team);

	float HUDHealth;
	float HUDMaxHelath;
	float HealthPercent;

	//**
	// Spectating
	//**
	UPROPERTY()
	class AHideGamePlayerState* HideGamePlayerState;

	UPROPERTY()
	class AHideGameState* HideGameState;

	int32 SepctatingIndex = 0;

	//**
	// Score
	//**

	UFUNCTION(Client, Reliable)
	void ClientShowPlayerScores(const TArray<FPlayerInfo>& Info);

	UPROPERTY(Replicated)
	bool bIsMatchEnd = false;

	UFUNCTION(Client, Reliable)
	void ClientShowPlayerWinners(const TArray<AHideGamePlayerState*>& Info);

	UPROPERTY()
	class UMultiCasualGameInstance* MultiCasualGameInstance;

public:

	FORCEINLINE bool IsLobby() { return bIsLobby; }
};
