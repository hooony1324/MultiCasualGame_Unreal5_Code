

#include "HideGameGameMode.h"
#include "GameHUD.h"
#include "MultiCasualPlayerController.h"
#include "MultiCasualGameCharacter.h"
#include "HideGamePlayerState.h"
#include "HideGameState.h"
#include "MultiCasualGameInstance.h"
#include "MultiCasual.h"

namespace MatchState
{
	const FName Hiding = FName("Hiding");
	const FName Seeking = FName("Seeking");
	const FName Cooldown = FName("Cooldown");
}

AHideGameGameMode::AHideGameGameMode()
{
	//HUDClass = AGameHUD::StaticClass();
	//PlayerControllerClass = AMultiCasualPlayerController::StaticClass();

	bDelayedStart = true;


}

void AHideGameGameMode::BeginPlay()
{
	Super::BeginPlay();


	MultiCasualGameInstance = Cast<UMultiCasualGameInstance>(GetGameInstance());

	MultiCasualGameInstance->SetMaxStageIndex(MaxStageIndex);

	bRestartTriggered = false;

	CountingWramupTime = 0.f;
	CountingWramupTime = 0.f;
	ScoringTime = 0.f;

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AHideGameGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	LivePlayersCount++;

	if (MultiCasualGameInstance)
	{
		if (LivePlayersCount == MultiCasualGameInstance->GetPlayersCount())
		{
			LevelStartingTime = GetWorld()->GetTimeSeconds();
		}
	}

}

void AHideGameGameMode::Logout(AController* Exiting)
{

}

void AHideGameGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		// Packaged에서만 작동
		CountingWramupTime += DeltaTime;
		if (LivePlayersCount == MultiCasualGameInstance->GetPlayersCount())
		{
			WarmupTime = CountingWramupTime;
			StartMatch();
			return;
		}

		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + InProgressTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Hiding);
		}

	}
	else if (MatchState == MatchState::Hiding)
	{
		CountdownTime = WarmupTime + InProgressTime + HidingTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Seeking);
		}
	}
	else if (MatchState == MatchState::Seeking)
	{
		CountdownTime = WarmupTime + InProgressTime + HidingTime + SeekingTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		CountSeekingTime += DeltaTime;

		// 플레이어 다 찾아서 종료
		if (LivePlayersCount == 1 && CountdownTime >= 0.0f)
		{
			ScoringTime += DeltaTime;
			if (ScoringTime < 3.0f)
			{
				return;
			}

			ScoringTime = 0.f;
			SeekingTime = CountSeekingTime;
			CountdownTime = WarmupTime + InProgressTime + HidingTime + SeekingTime + CoolDownTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
			SetMatchState(MatchState::Cooldown);
			return;
		}

		// 시간제한 종료
		if (CountdownTime < 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = WarmupTime + InProgressTime + HidingTime + SeekingTime + CoolDownTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}

}

void AHideGameGameMode::PlayerEliminated(AMultiCasualGameCharacter* ElimmedCharacter, AMultiCasualPlayerController* VictimController, AMultiCasualPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	// Attacker, ElimmedPlayer 처리
	// Attacker 점수 얻음
	AttackerPlayerState = AttackerController ? Cast<AHideGamePlayerState>(AttackerController->PlayerState) : nullptr;
	VictimPlayerState = VictimController ? Cast<AHideGamePlayerState>(VictimController->PlayerState) : nullptr;

	AHideGameState* HideGameState = GetGameState<AHideGameState>();

	if (AttackerPlayerState && (AttackerPlayerState != VictimPlayerState) && HideGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		HideGameState->UpdateTopScore(AttackerPlayerState);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	LivePlayersCount--;

}

void AHideGameGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMultiCasualPlayerController* PlayerController = Cast<AMultiCasualPlayerController>(*It);
		if (PlayerController)
		{
			PlayerController->OnMatchStateSet(MatchState);
		}

	}
}


void AHideGameGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	// 술래 선택 로직
	AHideGameState* HGameState = GetGameState<AHideGameState>();
	HGameState->EmptySpectatedPlayers();

	if (HGameState && MultiCasualGameInstance)
	{
		int32 PlayerNum = GameState->PlayerArray.Num();
		int32 SeekerIndex = (MultiCasualGameInstance->GetCurrentStageIndex() - 1) % PlayerNum;
		SeekerIndex = FMath::Clamp(SeekerIndex, 0, PlayerNum);

		for (int32 i = 0; i < PlayerNum; i++)
		{
			AHideGamePlayerState* PlayerState = Cast<AHideGamePlayerState>(GameState->PlayerArray[i].Get());
			if (PlayerState)
			{
				if (i == SeekerIndex)
				{
					// GameState->PlayerArray.Num() - 1
					PlayerState->SetTeam(ETeam::ET_SeekTeam);
					HGameState->SetSeeker(PlayerState);
				}
				else
				{
					PlayerState->SetTeam(ETeam::ET_HideTeam);
				}

				HGameState->AddSpectatedPlayer(Cast<AMultiCasualGameCharacter>(PlayerState->GetOwningController()->GetCharacter()));

				// Scores Saving
				FPlayerInfo SavingInfo;
				SavingInfo.PlayerName = PlayerState->GetPlayerName();
				SavingInfo.PlayerScore = PlayerState->GetScore();
				MultiCasualGameInstance->UpdatePlayerInfo(SavingInfo);
			}
		}
	}
}

void AHideGameGameMode::RestartGame()
{
	Super::RestartGame();
	
	MultiCasualGameInstance->AddStageIndex();

}

