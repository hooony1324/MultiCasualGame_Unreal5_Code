// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiCasualPlayerController.h"
#include "LobbyHUD.h"
#include "GameHUD.h"
#include "Chat.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "LobbyGameMode.h"
#include "MultiCasualGameCharacter.h"
#include "OverheadWidget.h"
#include "HideGameWidget.h"
#include "MatchAnnouncement.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Net/UnrealNetwork.h" 
#include "HideGameGameMode.h"
#include "HideGamePlayerState.h"
#include "HideGameTeam.h"
#include "CombatComponent.h"
#include "HideGameState.h"
#include "GameFramework/PlayerInput.h"
#include "MultiCasualGameInstance.h"
#include "Camera/CameraComponent.h"
#include "MultiCasual.h"

void AMultiCasualPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bIsLobby = nullptr == Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this)) ? false : true;

	GameHUD = Cast<AGameHUD>(GetHUD());

	ServerCheckMatchState();
	SetupInputComponent();

	InputComponent->BindAction("Enter", IE_Pressed, this, &AMultiCasualPlayerController::FocusChat);

	MultiCasualGameInstance = Cast<UMultiCasualGameInstance>(GetGameInstance());
}


void AMultiCasualPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiCasualPlayerController, MatchState);
	DOREPLIFETIME(AMultiCasualPlayerController, bIsMatchEnd);
}

void AMultiCasualPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);


}

// PRC를 받을 때, 생명주기중 가장 최초의 순간
void AMultiCasualPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}


void AMultiCasualPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	PollInit();
	CheckTimeSync(DeltaTime);

}

void AMultiCasualPlayerController::PollInit()
{
	//if (IsLocalController())
	//{
	//	if (ChatWidget == nullptr)
	//	{
	//		ChatWidget = CreateWidget<UChat>(this, ChatWidgetClass);
	//		ChatWidget->AddToViewport();
	//	}
	//}

	if (OverheadWidget == nullptr)
	{
		PlayerCharacter = Cast<AMultiCasualGameCharacter>(GetCharacter());

		if (PlayerCharacter)
		{
			OverheadWidget = PlayerCharacter->GetOverheadWidget();
		}
	}
}


void AMultiCasualPlayerController::FocusChat()
{
	if (ChatWidget == nullptr) return;

	ChatWidget->SetIsEnabled(true);
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ChatWidget->GetChatInputTextObject());
	bShowMouseCursor = true;
	SetInputMode(InputMode);
}

void AMultiCasualPlayerController::FocusGame()
{
	if (ChatWidget == nullptr) return;
	ChatWidget->SetIsEnabled(false);
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
}

#pragma region Chatting
void AMultiCasualPlayerController::SendMessage(const FText& Text)
{
	if (ChatWidget == nullptr) return;

	ServerSendMessage(Text.ToString());

	// OverheadWidget Message
	if (PlayerCharacter)
	{
		FString Left, Right;
		if (Text.ToString().Split(TEXT(":"), &Left, &Right))
		{
			PlayerCharacter->ServerSetOverheadMessage(Right);
		}
	}

}

void AMultiCasualPlayerController::ServerSendMessage_Implementation(const FString& Message)
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), OutActors);

	FString PlayerMessage = PlayerState->GetPlayerName();
	PlayerMessage.Append(" : " + Message);

	for (AActor* Actor : OutActors)
	{
		AMultiCasualPlayerController* MCP_Controller = Cast<AMultiCasualPlayerController>(Actor);
		if (MCP_Controller)
		{
			MCP_Controller->ClientSendMessage(PlayerMessage);
		}
	}
}

void AMultiCasualPlayerController::ClientSendMessage_Implementation(const FString& Message)
{
	if (!IsLocalController()) return;

	ChatWidget->AddChatToHistory(Message);
}
#pragma endregion

#pragma region GamePlay

void AMultiCasualPlayerController::ServerCheckMatchState_Implementation()
{
	// 서버 머신에서 생성된 클라이언트 캐릭터의 세팅
	AHideGameGameMode* GameMode = Cast<AHideGameGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		MatchState = GameMode->GetMatchState();
		WarmupTime = GameMode->GetWarmupTime();
		ProgressTime = GameMode->GetProgressTime();
		HidingTime = GameMode->GetHidingTime();
		SeekingTime = GameMode->GetSeekingTime();
		CoolDownTime = GameMode->GetCoolDownTime();
		LevelStartingTime = GameMode->GetLevelStartingTime();


		ClientJoinMidgame(MatchState, WarmupTime, ProgressTime, HidingTime, SeekingTime, CoolDownTime, LevelStartingTime);
	}
}

void AMultiCasualPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Progress, float Hiding, float Seeking, float Cooldown, float LevelStarting)
{
	MatchState = StateOfMatch;
	WarmupTime = Warmup;
	ProgressTime = Progress;
	HidingTime = Hiding;
	SeekingTime = Seeking;
	CoolDownTime = Cooldown;
	LevelStartingTime = LevelStarting;
	OnMatchStateSet(MatchState);

	if (GameHUD && MatchState == MatchState::WaitingToStart)
	{
		ChatWidget = CreateWidget<UChat>(this, ChatWidgetClass);
		ChatWidget->AddToViewport();

		GameHUD->AddHideGameWidget();
		GameHUD->AddMatchAnnouncementWidget();
		SetWarmupHud();
	}
}

void AMultiCasualPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;



	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = LevelStartingTime + WarmupTime - GetServerTime();
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = LevelStartingTime + WarmupTime + ProgressTime - GetServerTime();
	}
	else if (MatchState == MatchState::Hiding)
	{
		TimeLeft = LevelStartingTime + WarmupTime + ProgressTime + HidingTime - GetServerTime();
	}
	else if (MatchState == MatchState::Seeking)
	{
		TimeLeft = LevelStartingTime + WarmupTime + ProgressTime + HidingTime + SeekingTime - GetServerTime();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = LevelStartingTime + WarmupTime + ProgressTime + HidingTime + SeekingTime + CoolDownTime - GetServerTime();
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);


	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(TimeLeft);
	}

	CountdownInt = SecondsLeft;
}


float AMultiCasualPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
}

//**
// Sync Match Time
// 1. 클라에서 ServerTime 요청 (SingleTripTime 생김)
// 2. 서버는 클라이언트가 요구한 시간 체크, ClientRPC로 서버의 시간 전송 (SingleTripTime 생김)
// 3. 클라가 받은 정보 : 요청한 시간, 요청을 받았을 때 서버의 시간 (+RoundTripTime = Single + Single)
// - 클라가 서버시간을 받을 때 서버시간 = 서버시간 + STT
// - STT는 RTT의 절반
//**

void AMultiCasualPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AMultiCasualPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float CurrentClientTime = GetWorld()->GetTimeSeconds();
	float RoundTripTime = CurrentClientTime - TimeOfClientRequest;
	SingleTripTime = RoundTripTime * 0.5f;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - CurrentClientTime;
}

void AMultiCasualPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}


void AMultiCasualPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	GameHUD = GameHUD == nullptr ? Cast<AGameHUD>(GetHUD()) : GameHUD;

	bool bHUDValid = GameHUD &&
		GameHUD->HideGameWidget &&
		GameHUD->HideGameWidget->GameTime;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f || MatchState == MatchState::Cooldown)
		{
			FString Zero("00:00");
			GameHUD->HideGameWidget->GameTime->SetText(FText::FromString(Zero));
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		GameHUD->HideGameWidget->GameTime->SetText(FText::FromString(CountdownText));
	}
}

void AMultiCasualPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	GameHUD = GameHUD == nullptr ? Cast<AGameHUD>(GetHUD()) : GameHUD;

	bool bHUDValid = GameHUD &&
		GameHUD->HideGameWidget &&
		GameHUD->HideGameWidget->HealthBar &&
		GameHUD->HideGameWidget->HealthAmountText;

	if (bHUDValid)
	{
		HealthPercent = Health / MaxHealth;
		GameHUD->HideGameWidget->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d %%"), FMath::Clamp(FMath::CeilToInt(HealthPercent * 100.f), 0, 100));
		GameHUD->HideGameWidget->HealthAmountText->SetText(FText::FromString(HealthText));
	}
	else
	{
		HUDHealth = Health;
		HUDMaxHelath = MaxHealth;
	}

}

void AMultiCasualPlayerController::SetHUDScore(float Score)
{
	GameHUD = GameHUD == nullptr ? Cast<AGameHUD>(GetHUD()) : GameHUD;

	bool bHUDValid = GameHUD &&
		GameHUD->HideGameWidget &&
		GameHUD->HideGameWidget->ScoreText;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		GameHUD->HideGameWidget->ScoreText->SetText(FText::FromString(ScoreText));
	}

}

// Match별 UI컨트롤
void AMultiCasualPlayerController::OnMatchStateSet(FName State)
{
	// 예시
	MatchState = State;

	if (MatchState == MatchState::WaitingToStart)
	{
		HandleWarmUpStarted();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleInProgressStarted();
	}
	else if (MatchState == MatchState::Hiding)
	{
		HandleHidingStarted();
	}
	else if (MatchState == MatchState::Seeking)
	{
		HandleSeekingStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldownStarted();
	}
}

void AMultiCasualPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::WaitingToStart)
	{
		HandleWarmUpStarted();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleInProgressStarted();
	}
	else if (MatchState == MatchState::Hiding)
	{
		HandleHidingStarted();
	}
	else if (MatchState == MatchState::Seeking)
	{
		HandleSeekingStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldownStarted();
	}
}

void AMultiCasualPlayerController::SetWarmupHud()
{
	GameHUD = GameHUD == nullptr ? Cast<AGameHUD>(GetHUD()) : GameHUD;

	bool bHUDValid = GameHUD &&
		GameHUD->HideGameWidget &&
		GameHUD->HideGameWidget->MatchStateText;

	if (bHUDValid)
	{
		FString MatchText("WarmUp");
		GameHUD->HideGameWidget->MatchStateText->SetText(FText::FromString(MatchText));

		if (GameHUD->MatchAnnouncement)
		{
			GameHUD->MatchAnnouncement->SetVisibility(ESlateVisibility::Visible);
			GameHUD->MatchAnnouncement->AnnounceMessage(FString(TEXT("플레이어 기다리는 중...")));
		}

	}
}

void AMultiCasualPlayerController::HandleWarmUpStarted()
{
	// 유저 들어오는거 기다리는 중
	SetWarmupHud();
}

void AMultiCasualPlayerController::HandleInProgressStarted()
{
	// 술래 고르는 로직 진행
	// 이 Handler 직후에 캐릭터 소환됨
	bool bHUDValid = GameHUD &&
		GameHUD->HideGameWidget &&
		GameHUD->HideGameWidget->HealthBar &&
		GameHUD->HideGameWidget->HealthAmountText;

	if (bHUDValid)
	{
		SetHUDHealth(100, 100);
		GameHUD->MatchAnnouncement->SeekerSelecting();
	}

	if (HasAuthority() && MultiCasualGameInstance)
	{
		bIsMatchEnd = MultiCasualGameInstance->IsLastStage();
	}

}

void AMultiCasualPlayerController::HandleHidingStarted()
{
	GameHUD = GameHUD == nullptr ? Cast<AGameHUD>(GetHUD()) : GameHUD;


	HideGamePlayerState = HideGamePlayerState == nullptr ? GetPlayerState<AHideGamePlayerState>() : HideGamePlayerState;
	HideGameState = HideGameState == nullptr ? Cast<AHideGameState>(UGameplayStatics::GetGameState(this)) : HideGameState;

	bool bHUDValid = GameHUD &&
		GameHUD->HideGameWidget &&
		GameHUD->HideGameWidget->MatchStateText &&
		GameHUD->HideGameWidget->PlayerStateText;

	if (bHUDValid)
	{
		FString MatchText("Hiding");
		GameHUD->HideGameWidget->MatchStateText->SetText(FText::FromString(MatchText));
		
		if (HideGamePlayerState)
		{
			ETeam PlayerTeam = HideGamePlayerState->GetTeam();
			const UEnum* StateEnumType = FindObject<UEnum>(nullptr, TEXT("/Script/MultiCasual.ETeam"));
			if (StateEnumType)
			{
				FString TeamString = StateEnumType->GetDisplayNameTextByValue((int64)PlayerTeam).ToString();
				GameHUD->HideGameWidget->PlayerStateText->SetText(FText::FromString(TeamString));
			}

			if (PlayerCharacter && PlayerCharacter->GetCombatComponent())
			{
				PlayerCharacter->GetCombatComponent()->SetCurrentTeam(PlayerTeam);
				SetHUDHealth(PlayerCharacter->GetHealth(), PlayerCharacter->GetMaxHealth());
			}
		}

		// HideTeam Players Start Play
		if (HideGamePlayerState != HideGameState->GetSeeker())
		{
			GameHUD->HideGameWidget->EnableBlindfold(false);
			
			// 술래 Announcement
			if (HideGameState->GetSeeker())
			{
				FString SeekerName = HideGameState->GetSeeker()->GetPlayerName();
				GameHUD->MatchAnnouncement->HideStart(SeekerName);
			}
		}


	}
}

void AMultiCasualPlayerController::HandleSeekingStarted()
{
	GameHUD = GameHUD == nullptr ? Cast<AGameHUD>(GetHUD()) : GameHUD;

	HideGamePlayerState = HideGamePlayerState == nullptr ? GetPlayerState<AHideGamePlayerState>() : HideGamePlayerState;
	HideGameState = HideGameState == nullptr ? Cast<AHideGameState>(UGameplayStatics::GetGameState(this)) : HideGameState;

	bool bHUDValid = GameHUD &&
		GameHUD->HideGameWidget &&
		GameHUD->HideGameWidget->MatchStateText;

	if (bHUDValid)
	{
		FString MatchText("Seeking");
		GameHUD->HideGameWidget->MatchStateText->SetText(FText::FromString(MatchText));
		

		// SeekTeam Players Start Play
		if (HideGamePlayerState == HideGameState->GetSeeker())
		{
			GameHUD->HideGameWidget->EnableBlindfold(false);

			if (PlayerCharacter->GetCombatComponent())
			{
				SetCombatMode(ETeam::ET_SeekTeam);
			}
		}
		else
		{
			if (GameHUD->MatchAnnouncement)
			{
				GameHUD->MatchAnnouncement->AnnounceMessage(FString(TEXT("술래가 움직이기 시작합니다")));
			}
		}
	}
}

void AMultiCasualPlayerController::HandleCooldownStarted()
{
	GameHUD = GameHUD == nullptr ? Cast<AGameHUD>(GetHUD()) : GameHUD;

	bool bHUDValid = GameHUD &&
		GameHUD->HideGameWidget &&
		GameHUD->HideGameWidget->MatchStateText &&
		GameHUD->HideGameWidget->GameTime &&
		GameHUD->MatchAnnouncement;

	if (bHUDValid)
	{
		HideGameState = HideGameState == nullptr ? Cast<AHideGameState>(UGameplayStatics::GetGameState(this)) : HideGameState;
		HideGamePlayerState = HideGamePlayerState == nullptr ? GetPlayerState<AHideGamePlayerState>() : HideGamePlayerState;
		MultiCasualGameInstance = MultiCasualGameInstance == nullptr ? Cast<UMultiCasualGameInstance>(GetGameInstance()) : MultiCasualGameInstance;
		
		FString MatchText("Cooldown");
		GameHUD->HideGameWidget->MatchStateText->SetText(FText::FromString(MatchText));

		if (!bIsMatchEnd)
		{
			if (MultiCasualGameInstance)
			{
				// Save Scores
				for (int32 i = 0; i < HideGameState->PlayerArray.Num(); i++)
				{
					AHideGamePlayerState* CurPlayerState = Cast<AHideGamePlayerState>(HideGameState->PlayerArray[i].Get());

					FPlayerInfo SavingInfo;
					SavingInfo.PlayerName = CurPlayerState->GetPlayerName();
					SavingInfo.PlayerScore = CurPlayerState->GetScore();
					MultiCasualGameInstance->UpdatePlayerInfo(SavingInfo);
				}

				// Show Scores
				GameHUD->MatchAnnouncement->SetVisibility(ESlateVisibility::Visible);
				ClientShowPlayerScores(MultiCasualGameInstance->GetPlayersInfo());
			}
		}
		// Last Stage
		else
		{
			MatchText.Append(TEXT("_MatchEnd"));
			GameHUD->HideGameWidget->MatchStateText->SetText(FText::FromString(MatchText));
			GameHUD->MatchAnnouncement->SetVisibility(ESlateVisibility::Visible);
			ClientShowPlayerWinners(HideGameState->GetTopScoringPlayers());
		}

	}

}

void AMultiCasualPlayerController::ClientShowPlayerScores_Implementation(const TArray<FPlayerInfo>& Info)
{
	GameHUD->MatchAnnouncement->ShowScores(Info);
}

void AMultiCasualPlayerController::ClientShowPlayerWinners_Implementation(const TArray<AHideGamePlayerState*>& Info)
{
	GameHUD->MatchAnnouncement->ShowScores_MatchEnd(HideGameState->GetTopScoringPlayers());
}

void AMultiCasualPlayerController::SetCombatMode(ETeam Team)
{
	if (!PlayerCharacter->GetCombatComponent())
	{
		return;
	}

	switch (Team)
	{
	case ETeam::ET_HideTeam:
		// 무기X
		break;
	case ETeam::ET_SeekTeam:
		PlayerCharacter->GetCombatComponent()->ServerSpawnDefaultWeapon();
		break;
	case ETeam::ET_NoTeam:
		break;
	}
}

void AMultiCasualPlayerController::SepectateNextPlayer()
{
	HideGameState = HideGameState == nullptr ? Cast<AHideGameState>(UGameplayStatics::GetGameState(this)) : HideGameState;
	TArray<AMultiCasualGameCharacter*> Players = HideGameState->GetSpectatedPlayers();

	if (Players.Num() == 0)
	{
		return;
	}

	int32 N = Players.Num();
	while (true)
	{
		int32 NewIndex = (++SepctatingIndex % N + N) % N;
		
		if (!Players[NewIndex]->IsElimmed())
		{
			SepctatingIndex = NewIndex;
			return;
		}
	}

}

void AMultiCasualPlayerController::SepectatePrevPlayer()
{
	HideGameState = HideGameState == nullptr ? Cast<AHideGameState>(UGameplayStatics::GetGameState(this)) : HideGameState;
	TArray<AMultiCasualGameCharacter*> Players = HideGameState->GetSpectatedPlayers();

	if (Players.Num() == 0)
	{
		return;
	}

	int32 N = Players.Num();
	while (true)
	{
		int32 NewIndex = (--SepctatingIndex % N + N) % N;

		if (!Players[NewIndex]->IsElimmed())
		{
			SepctatingIndex = NewIndex;
			return;
		}
	}
}

void AMultiCasualPlayerController::Destroyed()
{
	if (GameHUD)
	{
		GameHUD->ClearAllWidgets();
	}
}


#pragma endregion