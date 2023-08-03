// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiCasualGameMode.h"
#include "HideGameGameMode.generated.h"

namespace MatchState
{
	extern MULTICASUAL_API const FName Hiding;
	extern MULTICASUAL_API const FName Seeking;
	extern MULTICASUAL_API const FName Cooldown;
}

/**
 * 
 */
UCLASS()
class MULTICASUAL_API AHideGameGameMode : public AMultiCasualGameMode
{
	GENERATED_BODY()
public:
	AHideGameGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class AMultiCasualGameCharacter* ElimmedCharacter, class AMultiCasualPlayerController* VictimController, AMultiCasualPlayerController* AttackerController);
	//virtual void ReuestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);


protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void OnMatchStateSet() override;

	virtual void HandleMatchHasStarted() override; // 술래 선택 로직
	virtual void RestartGame() override;


private:
	UPROPERTY(EditDefaultsOnly)
	int32 MaxStageIndex = 2;

	UPROPERTY(EditDefaultsOnly)		// 첫 매치가 시작하기전 유저가 들어오는 걸 기다리는 시간
	float WarmupTime = 60.f;
	float CountingWramupTime = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float InProgressTime = 4.f;		// 술래 정하고 모든 유저의 팀이 세팅되는 시간

	UPROPERTY(EditDefaultsOnly)
	float HidingTime = 15.f;

	UPROPERTY(EditDefaultsOnly)
	float SeekingTime = 180.f;
	float CountSeekingTime = 0.f;
	float ScoringTime = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float CoolDownTime = 6.f;

	float LevelStartingTime = 0.f;
	float CountdownTime = 0.f;

	UPROPERTY()
	class AHideGamePlayerState* AttackerPlayerState;

	UPROPERTY()
	class AHideGamePlayerState* VictimPlayerState;

	UPROPERTY()
	class UMultiCasualGameInstance* MultiCasualGameInstance;

	uint32 LivePlayersCount = 0;

	bool bRestartTriggered = false;
public:
	FORCEINLINE float GetWarmupTime() const { return WarmupTime; }
	FORCEINLINE float GetProgressTime() const { return InProgressTime; }
	FORCEINLINE float GetHidingTime() const { return HidingTime; }
	FORCEINLINE float GetSeekingTime() const { return SeekingTime; }
	FORCEINLINE float GetCoolDownTime() const { return CoolDownTime; }
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	FORCEINLINE float GetLevelStartingTime() const { return LevelStartingTime; }


};
