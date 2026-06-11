#include "ShootUserWidget.h"

#include "CPlayer.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "ShootGameMode.h"

#define BIND_SHOOT_BUTTON(WidgetName, FunctionName) \
	if (UButton* Button = Cast<UButton>(GetWidgetFromName(TEXT(WidgetName)))) \
	{ \
		Button->OnClicked.AddUniqueDynamic(this, &UShootUserWidget::FunctionName); \
	}

void UShootUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BIND_SHOOT_BUTTON("Lobby_PlayButton", OpenShipSelect);
	BIND_SHOOT_BUTTON("Lobby_DashboardButton", OpenDashboard);
	BIND_SHOOT_BUTTON("Lobby_ExitButton", QuitGame);

	BIND_SHOOT_BUTTON("Dashboard_BackButton", OpenLobby);

	BIND_SHOOT_BUTTON("Ship_FalconButton", SelectFalcon);
	BIND_SHOOT_BUTTON("Ship_TitanButton", SelectTitan);
	BIND_SHOOT_BUTTON("Ship_LaunchButton", StartSelectedGame);
	BIND_SHOOT_BUTTON("Ship_BackButton", OpenLobby);

	BIND_SHOOT_BUTTON("Pause_VolumeDownButton", DecreaseMasterVolume);
	BIND_SHOOT_BUTTON("Pause_VolumeUpButton", IncreaseMasterVolume);
	BIND_SHOOT_BUTTON("Pause_ResumeButton", ClosePauseMenu);

	BIND_SHOOT_BUTTON("GameOver_RestartButton", RestartGame);
	BIND_SHOOT_BUTTON("GameOver_LobbyButton", RestartGame);

	UpdateRuntimeText();
}

void UShootUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateRuntimeText();
}

void UShootUserWidget::OpenLobby()
{
	if (AShootGameMode* GameMode = GetShootGameMode())
	{
		GameMode->OpenLobby();
	}
}

void UShootUserWidget::OpenDashboard()
{
	if (AShootGameMode* GameMode = GetShootGameMode())
	{
		GameMode->OpenDashboard();
	}
}

void UShootUserWidget::OpenShipSelect()
{
	if (AShootGameMode* GameMode = GetShootGameMode())
	{
		GameMode->OpenShipSelect();
	}
}

void UShootUserWidget::SelectFalcon()
{
	if (AShootGameMode* GameMode = GetShootGameMode())
	{
		GameMode->SelectFalcon();
	}
}

void UShootUserWidget::SelectTitan()
{
	if (AShootGameMode* GameMode = GetShootGameMode())
	{
		GameMode->SelectTitan();
	}
}

void UShootUserWidget::StartSelectedGame()
{
	if (AShootGameMode* GameMode = GetShootGameMode())
	{
		GameMode->StartSelectedGame();
	}
}

void UShootUserWidget::RestartGame()
{
	if (AShootGameMode* GameMode = GetShootGameMode())
	{
		GameMode->RestartGame();
	}
}

void UShootUserWidget::QuitGame()
{
	if (AShootGameMode* GameMode = GetShootGameMode())
	{
		GameMode->QuitGame();
	}
}

void UShootUserWidget::ClosePauseMenu()
{
	if (AShootGameMode* GameMode = GetShootGameMode())
	{
		GameMode->ClosePauseMenu();
	}
}

void UShootUserWidget::IncreaseMasterVolume()
{
	if (AShootGameMode* GameMode = GetShootGameMode())
	{
		GameMode->IncreaseMasterVolume();
	}
}

void UShootUserWidget::DecreaseMasterVolume()
{
	if (AShootGameMode* GameMode = GetShootGameMode())
	{
		GameMode->DecreaseMasterVolume();
	}
}

EShootGameState UShootUserWidget::GetShootGameState() const
{
	const AShootGameMode* GameMode = GetShootGameMode();
	return GameMode ? GameMode->GetGameState() : EShootGameState::Lobby;
}

float UShootUserWidget::GetPlayerHealthRatio() const
{
	const ACPlayer* Player = GetShootPlayer();
	return Player ? Player->GetHealthRatio() : 0.0f;
}

float UShootUserWidget::GetPlayerHealth() const
{
	const ACPlayer* Player = GetShootPlayer();
	return Player ? Player->GetHealth() : 0.0f;
}

float UShootUserWidget::GetPlayerMaxHealth() const
{
	const ACPlayer* Player = GetShootPlayer();
	return Player ? Player->GetMaxHealth() : 0.0f;
}

int32 UShootUserWidget::GetPlayerScore() const
{
	const ACPlayer* Player = GetShootPlayer();
	return Player ? Player->GetScore() : 0;
}

FString UShootUserWidget::GetPlayerShipName() const
{
	const ACPlayer* Player = GetShootPlayer();
	return Player ? Player->GetShipName() : TEXT("");
}

int32 UShootUserWidget::GetCurrentWave() const
{
	const AShootGameMode* GameMode = GetShootGameMode();
	return GameMode ? GameMode->GetCurrentWave() : 1;
}

float UShootUserWidget::GetWaveProgressRatio() const
{
	const AShootGameMode* GameMode = GetShootGameMode();
	return GameMode ? GameMode->GetWaveProgressRatio() : 0.0f;
}

float UShootUserWidget::GetBossHealthRatio() const
{
	const AShootGameMode* GameMode = GetShootGameMode();
	return GameMode ? GameMode->GetBossHealthRatio() : 0.0f;
}

float UShootUserWidget::GetMasterVolume() const
{
	const AShootGameMode* GameMode = GetShootGameMode();
	return GameMode ? GameMode->GetMasterVolume() : 1.0f;
}

bool UShootUserWidget::IsBossActive() const
{
	const AShootGameMode* GameMode = GetShootGameMode();
	return GameMode && GameMode->IsBossActive();
}

bool UShootUserWidget::DidPlayerWin() const
{
	const AShootGameMode* GameMode = GetShootGameMode();
	return GameMode && GameMode->DidPlayerWin();
}

const TArray<FLeaderboardEntry>& UShootUserWidget::GetLeaderboardEntries() const
{
	static const TArray<FLeaderboardEntry> EmptyEntries;
	const AShootGameMode* GameMode = GetShootGameMode();
	return GameMode ? GameMode->GetLeaderboardEntries() : EmptyEntries;
}

void UShootUserWidget::UpdateRuntimeText()
{
	const ACPlayer* Player = GetShootPlayer();
	const AShootGameMode* GameMode = GetShootGameMode();

	if (Player)
	{
		SetTextByName(TEXT("Combat_HealthText"), FText::FromString(FString::Printf(TEXT("HP %.0f / %.0f"), Player->GetHealth(), Player->GetMaxHealth())));
		SetTextByName(TEXT("Combat_ScoreText"), FText::FromString(FString::Printf(TEXT("Score %d"), Player->GetScore())));
		SetTextByName(TEXT("Combat_ShipText"), FText::FromString(FString::Printf(TEXT("Ship %s"), *Player->GetShipName())));
		SetTextByName(TEXT("Combat_ViewText"), FText::FromString(Player->IsFirstPersonView() ? TEXT("View Cockpit") : TEXT("View Chase")));
		SetProgressByName(TEXT("Combat_HealthBar"), Player->GetHealthRatio());
	}

	if (GameMode)
	{
		const FString WaveText = GameMode->IsBossActive()
			? TEXT("FINAL WAVE")
			: FString::Printf(TEXT("Wave %d"), GameMode->GetCurrentWave());
		const bool IsBossActive = GameMode->IsBossActive();
		SetTextByName(TEXT("Combat_WaveText"), FText::FromString(WaveText));
		SetProgressByName(TEXT("Combat_WaveProgressBar"), GameMode->GetWaveProgressRatio());
		SetProgressByName(TEXT("Combat_BossHealthBar"), GameMode->GetBossHealthRatio());
		SetVisibilityByName(TEXT("Combat_WaveProgressLabelText"), IsBossActive ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
		SetVisibilityByName(TEXT("Combat_WaveProgressBar"), IsBossActive ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
		SetVisibilityByName(TEXT("Combat_BossHealthBar"), IsBossActive ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);

		const FString VolumeText = FString::Printf(TEXT("Master Volume %.0f%%"), GameMode->GetMasterVolume() * 100.0f);
		SetTextByName(TEXT("Pause_VolumeText"), FText::FromString(VolumeText));
		SetProgressByName(TEXT("Pause_VolumeBar"), GameMode->GetMasterVolume());

		SetTextByName(TEXT("GameOver_ResultText"), FText::FromString(GameMode->DidPlayerWin() ? TEXT("MISSION COMPLETE") : TEXT("MISSION FAILED")));
		const FText LeaderboardText = BuildLeaderboardText(GameMode->GetLeaderboardEntries());
		SetTextByName(TEXT("Dashboard_LeaderboardText"), LeaderboardText);
		SetTextByName(TEXT("GameOver_LeaderboardText"), LeaderboardText);

		if (Player)
		{
			const FString FinalText = FString::Printf(TEXT("Score %d   Wave %d   Ship %s   Time %.1fs"),
				Player->GetScore(),
				GameMode->GetCurrentWave(),
				*Player->GetShipName(),
				GameMode->GetSurvivedTime());
			SetTextByName(TEXT("GameOver_FinalText"), FText::FromString(FinalText));
		}
	}
}

FText UShootUserWidget::BuildLeaderboardText(const TArray<FLeaderboardEntry>& Entries) const
{
	if (Entries.Num() == 0)
	{
		return FText::FromString(TEXT("No records yet"));
	}

	FString Result;
	const int32 EntryCount = FMath::Min(Entries.Num(), 5);
	for (int32 Index = 0; Index < EntryCount; ++Index)
	{
		const FLeaderboardEntry& Entry = Entries[Index];
		Result += FString::Printf(
			TEXT("%d. %s  Score %d  Wave %d  %s  %.1fs"),
			Index + 1,
			*Entry.PlayerName,
			Entry.Score,
			Entry.Wave,
			Entry.DidWin ? TEXT("Win") : TEXT("Fail"),
			Entry.SurvivedTime);

		if (Index < EntryCount - 1)
		{
			Result += LINE_TERMINATOR;
		}
	}

	return FText::FromString(Result);
}

void UShootUserWidget::SetTextByName(FName WidgetName, const FText& Text)
{
	if (UTextBlock* TextBlock = Cast<UTextBlock>(GetWidgetFromName(WidgetName)))
	{
		TextBlock->SetText(Text);
	}
}

void UShootUserWidget::SetProgressByName(FName WidgetName, float Percent)
{
	if (UProgressBar* ProgressBar = Cast<UProgressBar>(GetWidgetFromName(WidgetName)))
	{
		ProgressBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}
}

void UShootUserWidget::SetVisibilityByName(FName WidgetName, ESlateVisibility NewVisibility)
{
	if (UWidget* Widget = GetWidgetFromName(WidgetName))
	{
		Widget->SetVisibility(NewVisibility);
	}
}

AShootGameMode* UShootUserWidget::GetShootGameMode() const
{
	return Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
}

ACPlayer* UShootUserWidget::GetShootPlayer() const
{
	return Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
}

#undef BIND_SHOOT_BUTTON
