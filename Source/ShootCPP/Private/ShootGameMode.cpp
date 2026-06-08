#include "ShootGameMode.h"

#include "BossEnemy.h"
#include "CPlayer.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LeaderboardSaveGame.h"
#include "ShootHUD.h"
#include "ShootPlayerController.h"
#include "Sound/SoundBase.h"
#include "SpaceArena.h"
#include "UObject/ConstructorHelpers.h"

AShootGameMode::AShootGameMode()
{
	DefaultPawnClass = ACPlayer::StaticClass();
	HUDClass = AShootHUD::StaticClass();
	PlayerControllerClass = AShootPlayerController::StaticClass();
	_enemyClass = AEnemy::StaticClass();
	_bossClass = ABossEnemy::StaticClass();

	ConfigureShipData();
	ConfigureWaveDesigns();
	_selectedShipData = _falconData;

	static ConstructorHelpers::FObjectFinder<USoundBase> LaunchSoundFinder(TEXT("/Script/Engine.SoundWave'/Engine/EditorSounds/GamePreview/StartPlayInEditor.StartPlayInEditor'"));
	if (LaunchSoundFinder.Succeeded())
	{
		_launchSound = LaunchSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> BossSoundFinder(TEXT("/Script/Engine.SoundWave'/Engine/VREditor/Sounds/VR_click3.VR_click3'"));
	if (BossSoundFinder.Succeeded())
	{
		_bossWarningSound = BossSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> WinSoundFinder(TEXT("/Script/Engine.SoundWave'/Engine/EditorSounds/Notifications/CompileSuccess.CompileSuccess'"));
	if (WinSoundFinder.Succeeded())
	{
		_winSound = WinSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> FailSoundFinder(TEXT("/Script/Engine.SoundWave'/Engine/EditorSounds/Notifications/CompileFailed.CompileFailed'"));
	if (FailSoundFinder.Succeeded())
	{
		_failSound = FailSoundFinder.Object;
	}
}

void AShootGameMode::BeginPlay()
{
	Super::BeginPlay();

	LoadLeaderboard();
	EnsureSpaceArena();
	ApplySelectedShipToPlayer();
	UpdateInputMode();
}

void AShootGameMode::OpenLobby()
{
	if (_gameState == EShootGameState::Playing)
	{
		return;
	}

	_gameState = EShootGameState::Lobby;
	UpdateInputMode();
}

void AShootGameMode::OpenDashboard()
{
	if (_gameState == EShootGameState::Playing)
	{
		return;
	}

	_gameState = EShootGameState::Dashboard;
	UpdateInputMode();
}

void AShootGameMode::OpenShipSelect()
{
	if (_gameState == EShootGameState::Playing)
	{
		return;
	}

	_gameState = EShootGameState::ShipSelect;
	ApplySelectedShipToPlayer();
	UpdateInputMode();
}

void AShootGameMode::SelectFalcon()
{
	if (_gameState != EShootGameState::ShipSelect)
	{
		return;
	}

	_selectedShipData = _falconData;
	ApplySelectedShipToPlayer();
}

void AShootGameMode::SelectTitan()
{
	if (_gameState != EShootGameState::ShipSelect)
	{
		return;
	}

	_selectedShipData = _titanData;
	ApplySelectedShipToPlayer();
}

void AShootGameMode::StartSelectedGame()
{
	if (_gameState != EShootGameState::ShipSelect)
	{
		return;
	}

	_gameState = EShootGameState::Playing;
	_currentWave = 1;
	_didPlayerWin = false;
	_playStartTime = GetWorld()->GetTimeSeconds();

	ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (Player)
	{
		Player->SetActorLocation(FVector::ZeroVector + FVector(0.0f, 0.0f, 360.0f));
		Player->SetActorRotation(FRotator::ZeroRotator);
		Player->ResetInputState();
		Player->ApplyShipData(_selectedShipData);
	}

	StartWave();
	UpdateInputMode();

	if (_launchSound)
	{
		UGameplayStatics::PlaySound2D(this, _launchSound, 0.55f);
	}
}

void AShootGameMode::RestartGame()
{
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

void AShootGameMode::QuitGame()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, true);
}

void AShootGameMode::HandlePrimaryClick(float ScreenX, float ScreenY)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	int32 ViewportWidth = 1280;
	int32 ViewportHeight = 720;
	if (PlayerController)
	{
		PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);
	}

	const FVector2D ViewportSize(ViewportWidth, ViewportHeight);
	const float CenterX = ViewportSize.X * 0.5f;

	if (_gameState == EShootGameState::Lobby)
	{
		if (IsInsideRect(ScreenX, ScreenY, CenterX - 150.0f, 250.0f, 300.0f, 48.0f))
		{
			OpenShipSelect();
			return;
		}

		if (IsInsideRect(ScreenX, ScreenY, CenterX - 150.0f, 316.0f, 300.0f, 48.0f))
		{
			OpenDashboard();
			return;
		}

		if (IsInsideRect(ScreenX, ScreenY, CenterX - 150.0f, 382.0f, 300.0f, 48.0f))
		{
			QuitGame();
			return;
		}
	}

	if (_gameState == EShootGameState::Dashboard)
	{
		if (IsInsideRect(ScreenX, ScreenY, CenterX - 120.0f, ViewportSize.Y - 110.0f, 240.0f, 46.0f))
		{
			OpenLobby();
			return;
		}
	}

	if (_gameState == EShootGameState::ShipSelect)
	{
		const float CardWidth = 330.0f;
		if (IsInsideRect(ScreenX, ScreenY, CenterX - CardWidth - 26.0f, 190.0f, CardWidth, 220.0f))
		{
			SelectFalcon();
			return;
		}

		if (IsInsideRect(ScreenX, ScreenY, CenterX + 26.0f, 190.0f, CardWidth, 220.0f))
		{
			SelectTitan();
			return;
		}

		if (IsInsideRect(ScreenX, ScreenY, CenterX - 150.0f, 452.0f, 300.0f, 48.0f))
		{
			StartSelectedGame();
			return;
		}

		if (IsInsideRect(ScreenX, ScreenY, CenterX - 120.0f, 516.0f, 240.0f, 42.0f))
		{
			OpenLobby();
			return;
		}
	}

	if (_gameState == EShootGameState::GameOver)
	{
		if (IsInsideRect(ScreenX, ScreenY, CenterX - 255.0f, ViewportSize.Y - 108.0f, 230.0f, 46.0f))
		{
			RestartGame();
			return;
		}

		if (IsInsideRect(ScreenX, ScreenY, CenterX + 25.0f, ViewportSize.Y - 108.0f, 230.0f, 46.0f))
		{
			RestartGame();
			return;
		}
	}
}

void AShootGameMode::RegisterEnemyKilled(int32 ScoreValue, bool IsBoss)
{
	ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (Player)
	{
		Player->AddScore(ScoreValue);
	}

	if (IsBoss)
	{
		EndGame(true);
	}
}

void AShootGameMode::EndGame(bool DidWin)
{
	if (_gameState == EShootGameState::GameOver)
	{
		return;
	}

	_gameState = EShootGameState::GameOver;
	_didPlayerWin = DidWin;
	GetWorldTimerManager().ClearTimer(_enemySpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(_waveTimerHandle);
	SaveLeaderboardEntry(DidWin);
	UpdateInputMode();

	USoundBase* ResultSound = DidWin ? _winSound : _failSound;
	if (ResultSound)
	{
		UGameplayStatics::PlaySound2D(this, ResultSound, 0.75f);
	}
}

EShootGameState AShootGameMode::GetGameState() const
{
	return _gameState;
}

const FPlayerShipData& AShootGameMode::GetSelectedShipData() const
{
	return _selectedShipData;
}

const TArray<FLeaderboardEntry>& AShootGameMode::GetLeaderboardEntries() const
{
	return _leaderboardEntries;
}

bool AShootGameMode::IsGameOver() const
{
	return _gameState == EShootGameState::GameOver;
}

bool AShootGameMode::DidPlayerWin() const
{
	return _didPlayerWin;
}

bool AShootGameMode::IsBossActive() const
{
	return IsValid(_bossEnemy) && _gameState == EShootGameState::Playing;
}

float AShootGameMode::GetBossHealthRatio() const
{
	if (!IsValid(_bossEnemy))
	{
		return 0.0f;
	}

	return _bossEnemy->GetHealthRatio();
}

int32 AShootGameMode::GetCurrentWave() const
{
	return _currentWave;
}

float AShootGameMode::GetWaveProgressRatio() const
{
	if (_gameState != EShootGameState::Playing || IsBossActive())
	{
		return 1.0f;
	}

	const float Elapsed = GetWorld()->GetTimeSeconds() - _waveStartTime;
	return FMath::Clamp(Elapsed / _waveDuration, 0.0f, 1.0f);
}

float AShootGameMode::GetSurvivedTime() const
{
	if (_playStartTime <= 0.0f)
	{
		return 0.0f;
	}

	return GetWorld()->GetTimeSeconds() - _playStartTime;
}

void AShootGameMode::ConfigureShipData()
{
	_falconData.ShipType = EPlayerShipType::Falcon;
	_falconData.DisplayName = TEXT("Falcon");
	_falconData.CruiseSpeed = 640.0f;
	_falconData.StrafeSpeed = 1080.0f;
	_falconData.VerticalSpeed = 900.0f;
	_falconData.MaxHealth = 90.0f;
	_falconData.BulletDamage = 16.0f;
	_falconData.FireInterval = 0.08f;
	_falconData.ShipColor = FLinearColor(0.0f, 0.55f, 1.0f, 1.0f);

	_titanData.ShipType = EPlayerShipType::Titan;
	_titanData.DisplayName = TEXT("Titan");
	_titanData.CruiseSpeed = 500.0f;
	_titanData.StrafeSpeed = 760.0f;
	_titanData.VerticalSpeed = 650.0f;
	_titanData.MaxHealth = 170.0f;
	_titanData.BulletDamage = 34.0f;
	_titanData.FireInterval = 0.18f;
	_titanData.ShipColor = FLinearColor(1.0f, 0.22f, 0.04f, 1.0f);
}

void AShootGameMode::ConfigureWaveDesigns()
{
	_waveDesigns.Empty();

	FWaveDesign Wave1;
	Wave1.WaveNumber = 1;
	Wave1.SpawnInterval = 1.45f;
	Wave1.EnemiesPerSpawn = 1;
	Wave1.BasicEnemySpeed = 560.0f;
	Wave1.BasicEnemyHealth = 35.0f;
	Wave1.BasicEnemyDamage = 14.0f;
	_waveDesigns.Add(Wave1);

	FWaveDesign Wave2;
	Wave2.WaveNumber = 2;
	Wave2.SpawnInterval = 1.2f;
	Wave2.EnemiesPerSpawn = 1;
	Wave2.BasicEnemySpeed = 640.0f;
	Wave2.BasicEnemyHealth = 45.0f;
	Wave2.BasicEnemyDamage = 18.0f;
	_waveDesigns.Add(Wave2);

	FWaveDesign Wave3;
	Wave3.WaveNumber = 3;
	Wave3.SpawnInterval = 1.0f;
	Wave3.EnemiesPerSpawn = 2;
	Wave3.BasicEnemySpeed = 720.0f;
	Wave3.BasicEnemyHealth = 52.0f;
	Wave3.BasicEnemyDamage = 20.0f;
	Wave3.FastEnemyChance = 0.42f;
	_waveDesigns.Add(Wave3);

	FWaveDesign Wave4;
	Wave4.WaveNumber = 4;
	Wave4.SpawnInterval = 0.82f;
	Wave4.EnemiesPerSpawn = 2;
	Wave4.BasicEnemySpeed = 790.0f;
	Wave4.BasicEnemyHealth = 62.0f;
	Wave4.BasicEnemyDamage = 24.0f;
	Wave4.FastEnemyChance = 0.35f;
	Wave4.TankEnemyChance = 0.24f;
	_waveDesigns.Add(Wave4);

	FWaveDesign Wave5;
	Wave5.WaveNumber = 5;
	Wave5.SpawnInterval = 0.68f;
	Wave5.EnemiesPerSpawn = 3;
	Wave5.BasicEnemySpeed = 860.0f;
	Wave5.BasicEnemyHealth = 72.0f;
	Wave5.BasicEnemyDamage = 28.0f;
	Wave5.FastEnemyChance = 0.4f;
	Wave5.TankEnemyChance = 0.32f;
	_waveDesigns.Add(Wave5);

	FWaveDesign Wave6;
	Wave6.WaveNumber = 6;
	Wave6.IsBossWave = true;
	_waveDesigns.Add(Wave6);
}

void AShootGameMode::ApplySelectedShipToPlayer()
{
	ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (Player)
	{
		Player->ApplyShipData(_selectedShipData);
	}
}

void AShootGameMode::EnsureSpaceArena()
{
	if (IsValid(_spaceArena))
	{
		return;
	}

	_spaceArena = GetWorld()->SpawnActor<ASpaceArena>(ASpaceArena::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
}

void AShootGameMode::UpdateInputMode()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	if (_gameState == EShootGameState::Playing)
	{
		AShootPlayerController* ShootPlayerController = Cast<AShootPlayerController>(PlayerController);
		if (ShootPlayerController)
		{
			ShootPlayerController->ApplyGameInputMode();
			return;
		}

		PlayerController->bShowMouseCursor = false;
		PlayerController->bEnableClickEvents = false;
		PlayerController->bEnableMouseOverEvents = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
		return;
	}

	AShootPlayerController* ShootPlayerController = Cast<AShootPlayerController>(PlayerController);
	if (ShootPlayerController)
	{
		ShootPlayerController->ApplyMenuInputMode();
		return;
	}

	PlayerController->bShowMouseCursor = true;
	PlayerController->bEnableClickEvents = true;
	PlayerController->bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
}

bool AShootGameMode::IsInsideRect(float ScreenX, float ScreenY, float RectX, float RectY, float RectWidth, float RectHeight) const
{
	return ScreenX >= RectX && ScreenX <= RectX + RectWidth && ScreenY >= RectY && ScreenY <= RectY + RectHeight;
}

void AShootGameMode::StartWave()
{
	if (_gameState != EShootGameState::Playing)
	{
		return;
	}

	_waveStartTime = GetWorld()->GetTimeSeconds();

	GetWorldTimerManager().ClearTimer(_enemySpawnTimerHandle);
	GetWorldTimerManager().SetTimer(_enemySpawnTimerHandle, this, &AShootGameMode::SpawnEnemy, GetCurrentSpawnInterval(), true, 0.2f);

	GetWorldTimerManager().ClearTimer(_waveTimerHandle);
	GetWorldTimerManager().SetTimer(_waveTimerHandle, this, &AShootGameMode::AdvanceWave, _waveDuration, false);
}

void AShootGameMode::AdvanceWave()
{
	if (_gameState != EShootGameState::Playing)
	{
		return;
	}

	_currentWave++;

	if (GetCurrentWaveDesign().IsBossWave)
	{
		SpawnBoss();
		return;
	}

	StartWave();
}

void AShootGameMode::SpawnEnemy()
{
	if (_gameState != EShootGameState::Playing || !_enemyClass || IsBossActive())
	{
		return;
	}

	const FWaveDesign& WaveDesign = GetCurrentWaveDesign();
	for (int32 SpawnIndex = 0; SpawnIndex < WaveDesign.EnemiesPerSpawn; ++SpawnIndex)
	{
		FVector SpawnLocation = GetSpawnLocation(1550.0f + SpawnIndex * 170.0f);
		SpawnLocation.Y += FMath::RandRange(-180.0f, 180.0f);
		const FRotator SpawnRotation(0.0f, 180.0f, 0.0f);
		AEnemy* Enemy = GetWorld()->SpawnActor<AEnemy>(_enemyClass, SpawnLocation, SpawnRotation);
		if (!Enemy)
		{
			continue;
		}

		const float Roll = FMath::FRand();
		if (Roll < WaveDesign.TankEnemyChance)
		{
			Enemy->InitializeEnemy(
				WaveDesign.BasicEnemySpeed * 0.72f,
				WaveDesign.BasicEnemyHealth * 2.15f,
				WaveDesign.BasicEnemyDamage * 1.45f,
				260,
				FVector(1.65f, 1.65f, 1.05f),
				FLinearColor(0.95f, 0.72f, 0.12f, 1.0f));
			continue;
		}

		if (Roll < WaveDesign.TankEnemyChance + WaveDesign.FastEnemyChance)
		{
			Enemy->InitializeEnemy(
				WaveDesign.BasicEnemySpeed * 1.42f,
				WaveDesign.BasicEnemyHealth * 0.78f,
				WaveDesign.BasicEnemyDamage * 1.1f,
				170,
				FVector(0.86f, 0.86f, 0.5f),
				FLinearColor(1.0f, 0.08f, 0.1f, 1.0f));
			continue;
		}

		Enemy->InitializeEnemy(
			WaveDesign.BasicEnemySpeed,
			WaveDesign.BasicEnemyHealth,
			WaveDesign.BasicEnemyDamage,
			100,
			FVector(1.1f, 1.1f, 0.7f),
			FLinearColor(0.55f, 0.9f, 1.0f, 1.0f));
	}
}

void AShootGameMode::SpawnBoss()
{
	if (_gameState != EShootGameState::Playing || !_bossClass || IsValid(_bossEnemy))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(_enemySpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(_waveTimerHandle);
	ClearEnemies();

	const FVector SpawnLocation = GetSpawnLocation(1850.0f);
	const FRotator SpawnRotation(0.0f, 180.0f, 0.0f);
	_bossEnemy = GetWorld()->SpawnActor<ABossEnemy>(_bossClass, SpawnLocation, SpawnRotation);

	if (_bossWarningSound)
	{
		UGameplayStatics::PlaySound2D(this, _bossWarningSound, 0.85f);
	}
}

void AShootGameMode::ClearEnemies()
{
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(this, AEnemy::StaticClass(), Enemies);
	for (AActor* Enemy : Enemies)
	{
		if (Enemy && Enemy != _bossEnemy)
		{
			Enemy->Destroy();
		}
	}
}

void AShootGameMode::LoadLeaderboard()
{
	_leaderboardEntries.Empty();

	USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(TEXT("ShootCPPLeaderboard"), 0);
	ULeaderboardSaveGame* LeaderboardSaveGame = Cast<ULeaderboardSaveGame>(SaveGame);
	if (LeaderboardSaveGame)
	{
		_leaderboardEntries = LeaderboardSaveGame->Entries;
	}
}

void AShootGameMode::SaveLeaderboardEntry(bool DidWin)
{
	ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player)
	{
		return;
	}

	FLeaderboardEntry Entry;
	Entry.PlayerName = TEXT("ACE");
	Entry.ShipName = Player->GetShipName();
	Entry.Score = Player->GetScore();
	Entry.Wave = _currentWave;
	Entry.SurvivedTime = GetSurvivedTime();
	Entry.DidWin = DidWin;
	_leaderboardEntries.Add(Entry);
	SortAndTrimLeaderboard();

	ULeaderboardSaveGame* LeaderboardSaveGame = Cast<ULeaderboardSaveGame>(UGameplayStatics::CreateSaveGameObject(ULeaderboardSaveGame::StaticClass()));
	if (!LeaderboardSaveGame)
	{
		return;
	}

	LeaderboardSaveGame->Entries = _leaderboardEntries;
	UGameplayStatics::SaveGameToSlot(LeaderboardSaveGame, TEXT("ShootCPPLeaderboard"), 0);
}

void AShootGameMode::SortAndTrimLeaderboard()
{
	_leaderboardEntries.Sort([](const FLeaderboardEntry& A, const FLeaderboardEntry& B)
	{
		if (A.Score == B.Score)
		{
			return A.Wave > B.Wave;
		}

		return A.Score > B.Score;
	});

	if (_leaderboardEntries.Num() > MaxLeaderboardEntries)
	{
		_leaderboardEntries.SetNum(MaxLeaderboardEntries);
	}
}

float AShootGameMode::GetCurrentSpawnInterval() const
{
	return GetCurrentWaveDesign().SpawnInterval;
}

const FWaveDesign& AShootGameMode::GetCurrentWaveDesign() const
{
	const int32 Index = FMath::Clamp(_currentWave - 1, 0, _waveDesigns.Num() - 1);
	return _waveDesigns[Index];
}

FVector AShootGameMode::GetSpawnLocation(float ForwardOffset) const
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector PlayerLocation = Player ? Player->GetActorLocation() : FVector::ZeroVector;

	return FVector(
		PlayerLocation.X + ForwardOffset,
		FMath::RandRange(-760.0f, 760.0f),
		PlayerLocation.Z);
}
