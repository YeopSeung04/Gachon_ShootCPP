#include "ShootHUD.h"

#include "CPlayer.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "ShootGameMode.h"
#include "ShootUserWidget.h"

namespace
{
	// Canvas HUD fallback에서 클릭 가능한 영역을 구분하기 위한 이름이다.
	// UMG 위젯이 연결되어 있으면 이 HitBox 방식은 사용하지 않는다.
	const FName LobbyPlayHitBoxName(TEXT("Lobby_Play"));
	const FName LobbyDashboardHitBoxName(TEXT("Lobby_Dashboard"));
	const FName LobbyExitHitBoxName(TEXT("Lobby_Exit"));
	const FName DashboardBackHitBoxName(TEXT("Dashboard_Back"));
	const FName ShipFalconHitBoxName(TEXT("Ship_Falcon"));
	const FName ShipTitanHitBoxName(TEXT("Ship_Titan"));
	const FName ShipLaunchHitBoxName(TEXT("Ship_Launch"));
	const FName ShipBackHitBoxName(TEXT("Ship_Back"));
	const FName GameOverRestartHitBoxName(TEXT("GameOver_Restart"));
	const FName GameOverLobbyHitBoxName(TEXT("GameOver_Lobby"));
	const FName PauseVolumeDownHitBoxName(TEXT("Pause_VolumeDown"));
	const FName PauseVolumeUpHitBoxName(TEXT("Pause_VolumeUp"));
	const FName PauseResumeHitBoxName(TEXT("Pause_Resume"));
}

void AShootHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	AShootGameMode* GameMode = Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameMode)
	{
		return;
	}

	SyncWidgetForState(GameMode);
	if (IsUsingWidgetForState(GameMode->GetGameState()))
	{
		// UMG 화면이 있으면 UMG가 메인 UI를 맡고, HUD는 적 체력바/궁극기 표시 같은 오버레이만 그린다.
		switch (GameMode->GetGameState())
		{
		case EShootGameState::Playing:
		case EShootGameState::PauseMenu:
		case EShootGameState::GameOver:
			DrawEnemyHealthBars();
			DrawUltimateStatus(Player);
			break;
		default:
			break;
		}
		return;
	}

	// UMG가 없는 환경에서도 게임을 확인할 수 있도록 Canvas 기반 UI를 fallback으로 직접 그린다.
	switch (GameMode->GetGameState())
	{
	case EShootGameState::Lobby:
		DrawLobby(GameMode);
		break;
	case EShootGameState::Dashboard:
		DrawDashboard(GameMode);
		break;
	case EShootGameState::ShipSelect:
		DrawShipSelect(GameMode);
		break;
	case EShootGameState::Playing:
		DrawCombatHud(GameMode, Player);
		break;
	case EShootGameState::PauseMenu:
		DrawCombatHud(GameMode, Player);
		DrawPauseMenu(GameMode);
		break;
	case EShootGameState::GameOver:
		DrawCombatHud(GameMode, Player);
		DrawGameOver(GameMode, Player);
		break;
	default:
		break;
	}
}

void AShootHUD::NotifyHitBoxClick(FName BoxName)
{
	Super::NotifyHitBoxClick(BoxName);

	AShootGameMode* GameMode = Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameMode)
	{
		return;
	}

	if (IsUsingWidgetForState(GameMode->GetGameState()))
	{
		return;
	}

	switch (GameMode->GetGameState())
	{
	case EShootGameState::PauseMenu:
		HandlePauseClick(GameMode, BoxName);
		break;
	case EShootGameState::Lobby:
		HandleLobbyClick(GameMode, BoxName);
		break;
	case EShootGameState::Dashboard:
		HandleDashboardClick(GameMode, BoxName);
		break;
	case EShootGameState::ShipSelect:
		HandleShipSelectClick(GameMode, BoxName);
		break;
	case EShootGameState::GameOver:
		HandleGameOverClick(GameMode, BoxName);
		break;
	default:
		break;
	}
}

void AShootHUD::SyncWidgetForState(AShootGameMode* GameMode)
{
	if (!GameMode)
	{
		return;
	}

	const EShootGameState GameState = GameMode->GetGameState();
	const TSubclassOf<UShootUserWidget> WidgetClass = GetWidgetClassForState(GameState);
	if (!WidgetClass)
	{
		// 현재 상태에 연결된 UMG 클래스가 없으면 기존 위젯을 제거하고 Canvas drawing으로 넘어간다.
		if (_activeWidget)
		{
			_activeWidget->RemoveFromParent();
			_activeWidget = nullptr;
		}
		return;
	}

	if (_activeWidget && _activeWidgetState == GameState && _activeWidget->GetClass() == WidgetClass)
	{
		// 이미 현재 상태에 맞는 위젯을 쓰고 있으면 새로 만들지 않는다.
		return;
	}

	if (_activeWidget)
	{
		_activeWidget->RemoveFromParent();
		_activeWidget = nullptr;
	}

	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	}

	if (!PlayerController)
	{
		return;
	}

	_activeWidget = CreateWidget<UShootUserWidget>(PlayerController, WidgetClass);
	if (_activeWidget)
	{
		_activeWidget->AddToViewport();
		_activeWidgetState = GameState;
	}
}

TSubclassOf<UShootUserWidget> AShootHUD::GetWidgetClassForState(EShootGameState GameState) const
{
	switch (GameState)
	{
	case EShootGameState::Lobby:
		return LobbyWidgetClass;
	case EShootGameState::Dashboard:
		return DashboardWidgetClass;
	case EShootGameState::ShipSelect:
		return ShipSelectWidgetClass;
	case EShootGameState::Playing:
		return CombatWidgetClass;
	case EShootGameState::PauseMenu:
		return PauseMenuWidgetClass;
	case EShootGameState::GameOver:
		return GameOverWidgetClass;
	default:
		return nullptr;
	}
}

bool AShootHUD::IsUsingWidgetForState(EShootGameState GameState) const
{
	return GetWidgetClassForState(GameState) != nullptr;
}

void AShootHUD::DrawLobby(AShootGameMode* GameMode)
{
	DrawRect(FLinearColor(0.012f, 0.016f, 0.024f, 0.98f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

	DrawCenteredText(TEXT("DUAL ACE: SKY RIFT"), 92.0f, FLinearColor::White, GEngine->GetLargeFont(), 1.45f);
	DrawCenteredText(TEXT("C++ 3D Flight Shooting Game"), 150.0f, FLinearColor(0.62f, 0.82f, 1.0f, 1.0f), GEngine->GetMediumFont(), 0.9f);

	const float CenterX = Canvas->SizeX * 0.5f;
	DrawButton(CenterX - 150.0f, 250.0f, 300.0f, 48.0f, TEXT("Game Play"), FLinearColor(0.08f, 0.38f, 0.8f, 1.0f), LobbyPlayHitBoxName);
	DrawButton(CenterX - 150.0f, 316.0f, 300.0f, 48.0f, TEXT("Dashboard"), FLinearColor(0.08f, 0.26f, 0.38f, 1.0f), LobbyDashboardHitBoxName);
	DrawButton(CenterX - 150.0f, 382.0f, 300.0f, 48.0f, TEXT("Exit"), FLinearColor(0.42f, 0.08f, 0.08f, 1.0f), LobbyExitHitBoxName);

	DrawCenteredText(TEXT("Mouse click is enabled on menu screens."), 482.0f, FLinearColor(0.76f, 0.8f, 0.88f, 1.0f), GEngine->GetSmallFont(), 1.0f);
}

void AShootHUD::DrawDashboard(AShootGameMode* GameMode)
{
	DrawRect(FLinearColor(0.012f, 0.016f, 0.024f, 0.98f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

	DrawCenteredText(TEXT("DASHBOARD"), 82.0f, FLinearColor::White, GEngine->GetLargeFont(), 1.25f);
	DrawCenteredText(TEXT("Local Top 5 leaderboard"), 138.0f, FLinearColor(0.62f, 0.82f, 1.0f, 1.0f), GEngine->GetMediumFont(), 0.86f);
	DrawLeaderboard(GameMode->GetLeaderboardEntries(), Canvas->SizeX * 0.5f - 270.0f, 220.0f);
	DrawButton(Canvas->SizeX * 0.5f - 120.0f, Canvas->SizeY - 110.0f, 240.0f, 46.0f, TEXT("Back"), FLinearColor(0.08f, 0.26f, 0.38f, 1.0f), DashboardBackHitBoxName);
}

void AShootHUD::DrawShipSelect(AShootGameMode* GameMode)
{
	DrawRect(FLinearColor(0.015f, 0.018f, 0.025f, 0.96f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

	DrawCenteredText(TEXT("DUAL ACE: SKY RIFT"), 70.0f, FLinearColor::White, GEngine->GetLargeFont(), 1.35f);
	DrawCenteredText(TEXT("Choose your C++ fighter"), 125.0f, FLinearColor(0.62f, 0.82f, 1.0f, 1.0f), GEngine->GetMediumFont(), 0.95f);

	const float CardWidth = 330.0f;
	const float CardHeight = 220.0f;
	const float CenterX = Canvas->SizeX * 0.5f;
	const FPlayerShipData& SelectedShip = GameMode->GetSelectedShipData();

	FPlayerShipData FalconData;
	FalconData.DisplayName = TEXT("Falcon");
	FalconData.ShipType = EPlayerShipType::Falcon;
	FalconData.CruiseSpeed = 640.0f;
	FalconData.StrafeSpeed = 1080.0f;
	FalconData.MaxHealth = 90.0f;
	FalconData.BulletDamage = 16.0f;
	FalconData.FireInterval = 0.08f;
	FalconData.ShipColor = FLinearColor(0.0f, 0.55f, 1.0f, 1.0f);

	FPlayerShipData TitanData;
	TitanData.DisplayName = TEXT("Titan");
	TitanData.ShipType = EPlayerShipType::Titan;
	TitanData.CruiseSpeed = 500.0f;
	TitanData.StrafeSpeed = 760.0f;
	TitanData.MaxHealth = 170.0f;
	TitanData.BulletDamage = 34.0f;
	TitanData.FireInterval = 0.18f;
	TitanData.ShipColor = FLinearColor(1.0f, 0.22f, 0.04f, 1.0f);

	DrawShipCard(CenterX - CardWidth - 26.0f, 190.0f, CardWidth, CardHeight, FalconData, SelectedShip.ShipType == EPlayerShipType::Falcon, ShipFalconHitBoxName);
	DrawShipCard(CenterX + 26.0f, 190.0f, CardWidth, CardHeight, TitanData, SelectedShip.ShipType == EPlayerShipType::Titan, ShipTitanHitBoxName);

	DrawButton(CenterX - 150.0f, 452.0f, 300.0f, 48.0f, TEXT("Launch"), FLinearColor(0.08f, 0.38f, 0.8f, 1.0f), ShipLaunchHitBoxName);
	DrawButton(CenterX - 120.0f, 516.0f, 240.0f, 42.0f, TEXT("Back"), FLinearColor(0.08f, 0.26f, 0.38f, 1.0f), ShipBackHitBoxName);
	DrawCenteredText(TEXT("1 Falcon     2 Titan     Enter Launch"), 578.0f, FLinearColor::Yellow, GEngine->GetMediumFont(), 0.82f);
	DrawCenteredText(TEXT("V switches cockpit/chase view during combat."), 620.0f, FLinearColor(0.78f, 0.82f, 0.88f, 1.0f), GEngine->GetSmallFont(), 1.0f);

	DrawLeaderboard(GameMode->GetLeaderboardEntries(), CenterX - 260.0f, 660.0f);
}

void AShootHUD::DrawCombatHud(AShootGameMode* GameMode, ACPlayer* Player)
{
	const float Padding = 28.0f;
	DrawText(TEXT("DUAL ACE"), FLinearColor::White, Padding, Padding, GEngine->GetMediumFont(), 1.15f);

	if (Player)
	{
		DrawBar(Padding, 70.0f, 270.0f, 18.0f, Player->GetHealthRatio(), FLinearColor(0.1f, 0.85f, 0.42f, 1.0f), TEXT("Hull"));

		const FString HealthText = FString::Printf(TEXT("HP %.0f / %.0f"), Player->GetHealth(), Player->GetMaxHealth());
		DrawText(HealthText, FLinearColor(0.8f, 1.0f, 0.86f, 1.0f), Padding + 285.0f, 67.0f, GEngine->GetSmallFont(), 0.95f);

		const FString ScoreText = FString::Printf(TEXT("Score  %d"), Player->GetScore());
		DrawText(ScoreText, FLinearColor::White, Padding, 102.0f, GEngine->GetSmallFont(), 1.1f);

		const FString ShipText = FString::Printf(TEXT("Ship  %s"), *Player->GetShipName());
		DrawText(ShipText, FLinearColor(0.65f, 0.85f, 1.0f, 1.0f), Padding, 128.0f, GEngine->GetSmallFont(), 1.0f);

		const FString CameraText = Player->IsFirstPersonView() ? TEXT("View  Cockpit") : TEXT("View  Chase");
		DrawText(CameraText, FLinearColor(0.65f, 0.85f, 1.0f, 1.0f), Padding, 152.0f, GEngine->GetSmallFont(), 1.0f);
	}

	const FString WaveText = GameMode->IsBossActive()
		? TEXT("FINAL WAVE")
		: FString::Printf(TEXT("Wave  %d"), GameMode->GetCurrentWave());
	DrawCenteredText(WaveText, 36.0f, FLinearColor::White, GEngine->GetMediumFont(), 1.0f);

	if (!GameMode->IsBossActive())
	{
		DrawBar(Canvas->SizeX * 0.5f - 170.0f, 72.0f, 340.0f, 10.0f, GameMode->GetWaveProgressRatio(), FLinearColor(0.2f, 0.65f, 1.0f, 1.0f), TEXT("Wave Progress"));
	}

	if (GameMode->IsBossActive())
	{
		DrawBar(Canvas->SizeX * 0.5f - 250.0f, 72.0f, 500.0f, 20.0f, GameMode->GetBossHealthRatio(), FLinearColor(0.95f, 0.16f, 0.1f, 1.0f), TEXT("Command Carrier"));
	}

	const float CrossX = Canvas->SizeX * 0.5f;
	const float CrossY = Canvas->SizeY * 0.5f;
	DrawRect(FLinearColor::White, CrossX - 18.0f, CrossY, 36.0f, 1.0f);
	DrawRect(FLinearColor::White, CrossX, CrossY - 18.0f, 1.0f, 36.0f);

	DrawEnemyHealthBars();
	DrawUltimateStatus(Player);
}

void AShootHUD::DrawUltimateStatus(ACPlayer* Player)
{
	if (!Player)
	{
		return;
	}

	FString UltimateText = TEXT("R  ULT LOCKED");
	FLinearColor TextColor(0.55f, 0.58f, 0.64f, 1.0f);
	if (Player->IsUltimateReady())
	{
		UltimateText = FString::Printf(TEXT("R  ULT READY x%d"), Player->GetUltimateChargeCount());
		TextColor = FLinearColor(1.0f, 0.88f, 0.16f, 1.0f);
	}
	else if (Player->HasUsedUltimateThisWave())
	{
		UltimateText = TEXT("R  ULT USED");
	}

	DrawText(UltimateText, TextColor, Canvas->SizeX - 190.0f, Canvas->SizeY - 70.0f, GEngine->GetSmallFont(), 1.1f);
}

void AShootHUD::DrawPauseMenu(AShootGameMode* GameMode)
{
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.62f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

	const float PanelWidth = 460.0f;
	const float PanelHeight = 280.0f;
	const float PanelX = Canvas->SizeX * 0.5f - PanelWidth * 0.5f;
	const float PanelY = Canvas->SizeY * 0.5f - PanelHeight * 0.5f;

	DrawRect(FLinearColor(0.75f, 0.84f, 1.0f, 0.95f), PanelX, PanelY, PanelWidth, PanelHeight);
	DrawRect(FLinearColor(0.035f, 0.045f, 0.06f, 0.98f), PanelX + 3.0f, PanelY + 3.0f, PanelWidth - 6.0f, PanelHeight - 6.0f);

	DrawCenteredText(TEXT("PAUSED"), PanelY + 34.0f, FLinearColor::White, GEngine->GetLargeFont(), 1.05f);

	const float Volume = GameMode ? GameMode->GetMasterVolume() : 1.0f;
	const FString VolumeText = FString::Printf(TEXT("Master Volume  %.0f%%"), Volume * 100.0f);
	DrawCenteredText(VolumeText, PanelY + 96.0f, FLinearColor(0.78f, 0.9f, 1.0f, 1.0f), GEngine->GetMediumFont(), 0.82f);

	const float BarX = PanelX + 92.0f;
	const float BarY = PanelY + 142.0f;
	const float BarWidth = PanelWidth - 184.0f;
	DrawRect(FLinearColor(0.015f, 0.02f, 0.026f, 1.0f), BarX, BarY, BarWidth, 18.0f);
	DrawRect(FLinearColor(0.2f, 0.66f, 1.0f, 1.0f), BarX + 2.0f, BarY + 2.0f, (BarWidth - 4.0f) * FMath::Clamp(Volume, 0.0f, 1.0f), 14.0f);

	DrawButton(PanelX + 86.0f, PanelY + 178.0f, 90.0f, 42.0f, TEXT("-"), FLinearColor(0.08f, 0.26f, 0.38f, 1.0f), PauseVolumeDownHitBoxName);
	DrawButton(PanelX + PanelWidth - 176.0f, PanelY + 178.0f, 90.0f, 42.0f, TEXT("+"), FLinearColor(0.08f, 0.38f, 0.8f, 1.0f), PauseVolumeUpHitBoxName);
	DrawButton(PanelX + 130.0f, PanelY + 230.0f, 200.0f, 42.0f, TEXT("Resume"), FLinearColor(0.08f, 0.36f, 0.22f, 1.0f), PauseResumeHitBoxName);
}

void AShootHUD::DrawGameOver(AShootGameMode* GameMode, ACPlayer* Player)
{
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.74f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

	const FString ResultText = GameMode->DidPlayerWin() ? TEXT("MISSION COMPLETE") : TEXT("MISSION FAILED");
	DrawCenteredText(ResultText, 120.0f, FLinearColor::Yellow, GEngine->GetLargeFont(), 1.35f);

	if (Player)
	{
		const FString FinalText = FString::Printf(TEXT("Score %d   Wave %d   Ship %s   Time %.1fs"),
			Player->GetScore(),
			GameMode->GetCurrentWave(),
			*Player->GetShipName(),
			GameMode->GetSurvivedTime());
		DrawCenteredText(FinalText, 185.0f, FLinearColor::White, GEngine->GetMediumFont(), 0.85f);
	}

	DrawLeaderboard(GameMode->GetLeaderboardEntries(), Canvas->SizeX * 0.5f - 270.0f, 250.0f);
	DrawButton(Canvas->SizeX * 0.5f - 255.0f, Canvas->SizeY - 108.0f, 230.0f, 46.0f, TEXT("Restart"), FLinearColor(0.08f, 0.38f, 0.8f, 1.0f), GameOverRestartHitBoxName);
	DrawButton(Canvas->SizeX * 0.5f + 25.0f, Canvas->SizeY - 108.0f, 230.0f, 46.0f, TEXT("Lobby"), FLinearColor(0.08f, 0.26f, 0.38f, 1.0f), GameOverLobbyHitBoxName);
}

void AShootHUD::DrawLeaderboard(const TArray<FLeaderboardEntry>& Entries, float X, float Y)
{
	DrawRect(FLinearColor(0.04f, 0.05f, 0.07f, 0.86f), X, Y, 540.0f, 190.0f);
	DrawText(TEXT("LEADERBOARD"), FLinearColor::White, X + 22.0f, Y + 18.0f, GEngine->GetMediumFont(), 0.9f);

	if (Entries.Num() == 0)
	{
		DrawText(TEXT("No records yet"), FLinearColor(0.72f, 0.76f, 0.82f, 1.0f), X + 22.0f, Y + 62.0f, GEngine->GetSmallFont(), 1.0f);
		return;
	}

	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		const FLeaderboardEntry& Entry = Entries[Index];
		const FString Row = FString::Printf(TEXT("%d. %-3s  %6d  Wave %d  %s  %s"),
			Index + 1,
			*Entry.PlayerName,
			Entry.Score,
			Entry.Wave,
			*Entry.ShipName,
			Entry.DidWin ? TEXT("Win") : TEXT("Fail"));
		DrawText(Row, FLinearColor(0.86f, 0.9f, 0.96f, 1.0f), X + 22.0f, Y + 58.0f + Index * 25.0f, GEngine->GetSmallFont(), 0.95f);
	}
}

void AShootHUD::DrawEnemyHealthBars()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	TArray<AActor*> EnemyActors;
	UGameplayStatics::GetAllActorsOfClass(this, AEnemy::StaticClass(), EnemyActors);
	for (AActor* Actor : EnemyActors)
	{
		AEnemy* Enemy = Cast<AEnemy>(Actor);
		if (!Enemy || Enemy->GetHealth() <= 0.0f)
		{
			continue;
		}

		FVector2D ScreenLocation;
		const FVector WorldLocation = Enemy->GetActorLocation() + FVector(0.0f, 0.0f, 120.0f);
		if (!PlayerController->ProjectWorldLocationToScreen(WorldLocation, ScreenLocation))
		{
			continue;
		}

		// 3D 월드 좌표를 화면 좌표로 투영해 적 머리 위에 체력바를 그린다.
		const float Width = 78.0f;
		const float Height = 7.0f;
		const float X = ScreenLocation.X - Width * 0.5f;
		const float Y = ScreenLocation.Y;
		DrawRect(FLinearColor(0.02f, 0.02f, 0.025f, 0.85f), X, Y, Width, Height);
		DrawRect(FLinearColor(0.95f, 0.18f, 0.12f, 1.0f), X + 1.0f, Y + 1.0f, (Width - 2.0f) * Enemy->GetHealthRatio(), Height - 2.0f);

		const FString HealthText = FString::Printf(TEXT("%.0f"), Enemy->GetHealth());
		DrawText(HealthText, FLinearColor::White, X + Width + 5.0f, Y - 6.0f, GEngine->GetSmallFont(), 0.72f);
	}
}

void AShootHUD::DrawShipCard(float X, float Y, float Width, float Height, const FPlayerShipData& ShipData, bool IsSelected, FName HitBoxName)
{
	AddHitBox(FVector2D(X, Y), FVector2D(Width, Height), HitBoxName, true, 10);

	const FLinearColor BorderColor = IsSelected ? FLinearColor::Yellow : FLinearColor(0.28f, 0.34f, 0.42f, 1.0f);
	DrawRect(BorderColor, X, Y, Width, Height);
	DrawRect(FLinearColor(0.035f, 0.045f, 0.06f, 0.98f), X + 3.0f, Y + 3.0f, Width - 6.0f, Height - 6.0f);
	DrawRect(ShipData.ShipColor, X + 24.0f, Y + 36.0f, 86.0f, 20.0f);

	DrawText(ShipData.DisplayName, FLinearColor::White, X + 24.0f, Y + 72.0f, GEngine->GetMediumFont(), 1.0f);

	const FString SpeedText = FString::Printf(TEXT("Speed        %.0f"), ShipData.CruiseSpeed);
	const FString HealthText = FString::Printf(TEXT("Hull         %.0f"), ShipData.MaxHealth);
	const FString DamageText = FString::Printf(TEXT("Damage       %.0f"), ShipData.BulletDamage);
	const FString FireText = FString::Printf(TEXT("Fire Delay   %.2f"), ShipData.FireInterval);

	DrawText(SpeedText, FLinearColor(0.82f, 0.87f, 0.94f, 1.0f), X + 24.0f, Y + 112.0f, GEngine->GetSmallFont(), 0.95f);
	DrawText(HealthText, FLinearColor(0.82f, 0.87f, 0.94f, 1.0f), X + 24.0f, Y + 136.0f, GEngine->GetSmallFont(), 0.95f);
	DrawText(DamageText, FLinearColor(0.82f, 0.87f, 0.94f, 1.0f), X + 24.0f, Y + 160.0f, GEngine->GetSmallFont(), 0.95f);
	DrawText(FireText, FLinearColor(0.82f, 0.87f, 0.94f, 1.0f), X + 24.0f, Y + 184.0f, GEngine->GetSmallFont(), 0.95f);
}

void AShootHUD::DrawButton(float X, float Y, float Width, float Height, const FString& Text, const FLinearColor& FillColor, FName HitBoxName)
{
	// Canvas UI는 실제 Button 위젯이 없으므로 AddHitBox로 클릭 영역을 등록한다.
	AddHitBox(FVector2D(X, Y), FVector2D(Width, Height), HitBoxName, true, 20);

	DrawRect(FLinearColor(0.78f, 0.86f, 1.0f, 0.9f), X, Y, Width, Height);
	DrawRect(FillColor, X + 2.0f, Y + 2.0f, Width - 4.0f, Height - 4.0f);

	float TextWidth = 0.0f;
	float TextHeight = 0.0f;
	GetTextSize(Text, TextWidth, TextHeight, GEngine->GetMediumFont(), 0.82f);
	DrawText(Text, FLinearColor::White, X + Width * 0.5f - TextWidth * 0.5f, Y + Height * 0.5f - TextHeight * 0.5f, GEngine->GetMediumFont(), 0.82f);
}

void AShootHUD::HandleLobbyClick(AShootGameMode* GameMode, FName BoxName)
{
	if (BoxName == LobbyPlayHitBoxName)
	{
		GameMode->OpenShipSelect();
		return;
	}

	if (BoxName == LobbyDashboardHitBoxName)
	{
		GameMode->OpenDashboard();
		return;
	}

	if (BoxName == LobbyExitHitBoxName)
	{
		GameMode->QuitGame();
	}
}

void AShootHUD::HandleDashboardClick(AShootGameMode* GameMode, FName BoxName)
{
	if (BoxName == DashboardBackHitBoxName)
	{
		GameMode->OpenLobby();
	}
}

void AShootHUD::HandleShipSelectClick(AShootGameMode* GameMode, FName BoxName)
{
	if (BoxName == ShipFalconHitBoxName)
	{
		GameMode->SelectFalcon();
		return;
	}

	if (BoxName == ShipTitanHitBoxName)
	{
		GameMode->SelectTitan();
		return;
	}

	if (BoxName == ShipLaunchHitBoxName)
	{
		GameMode->StartSelectedGame();
		return;
	}

	if (BoxName == ShipBackHitBoxName)
	{
		GameMode->OpenLobby();
	}
}

void AShootHUD::HandlePauseClick(AShootGameMode* GameMode, FName BoxName)
{
	if (BoxName == PauseVolumeDownHitBoxName)
	{
		GameMode->DecreaseMasterVolume();
		return;
	}

	if (BoxName == PauseVolumeUpHitBoxName)
	{
		GameMode->IncreaseMasterVolume();
		return;
	}

	if (BoxName == PauseResumeHitBoxName)
	{
		GameMode->ClosePauseMenu();
	}
}

void AShootHUD::HandleGameOverClick(AShootGameMode* GameMode, FName BoxName)
{
	if (BoxName == GameOverRestartHitBoxName)
	{
		GameMode->RestartGame();
		return;
	}

	if (BoxName == GameOverLobbyHitBoxName)
	{
		GameMode->OpenLobby();
	}
}

void AShootHUD::DrawBar(float X, float Y, float Width, float Height, float Ratio, const FLinearColor& FillColor, const FString& Label)
{
	// 체력, 웨이브 진행도, 보스 체력처럼 0~1 비율로 표현되는 값을 공통 막대로 그린다.
	DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.82f), X, Y, Width, Height);
	DrawRect(FillColor, X + 2.0f, Y + 2.0f, (Width - 4.0f) * FMath::Clamp(Ratio, 0.0f, 1.0f), Height - 4.0f);
	DrawText(Label, FLinearColor::White, X, Y - 19.0f, GEngine->GetSmallFont(), 0.9f);
}

void AShootHUD::DrawCenteredText(const FString& Text, float Y, const FLinearColor& Color, UFont* Font, float Scale)
{
	float TextWidth = 0.0f;
	float TextHeight = 0.0f;
	GetTextSize(Text, TextWidth, TextHeight, Font, Scale);
	DrawText(Text, Color, Canvas->SizeX * 0.5f - TextWidth * 0.5f, Y, Font, Scale);
}
