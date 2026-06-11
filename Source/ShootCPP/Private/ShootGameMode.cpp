#include "ShootGameMode.h"

#include "BossEnemy.h"
#include "CPlayer.h"
#include "Components/AudioComponent.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LeaderboardSaveGame.h"
#include "ShootHUD.h"
#include "ShootPlayerController.h"
#include "Sound/AudioSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundBase.h"
#include "SpaceArena.h"
#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"

AShootGameMode::AShootGameMode()
{
	DefaultPawnClass = ACPlayer::StaticClass();
	HUDClass = AShootHUD::StaticClass();
	PlayerControllerClass = AShootPlayerController::StaticClass();
	_enemyClass = AEnemy::StaticClass();
	_bossClass = ABossEnemy::StaticClass();

	static ConstructorHelpers::FObjectFinder<UDataTable> PlayerStatTableFinder(TEXT("/Script/Engine.DataTable'/Game/Data/DT_PlayerStats.DT_PlayerStats'"));
	if (PlayerStatTableFinder.Succeeded())
	{
		_playerStatTable = PlayerStatTableFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> EnemyStatTableFinder(TEXT("/Script/Engine.DataTable'/Game/Data/DT_EnemyStats.DT_EnemyStats'"));
	if (EnemyStatTableFinder.Succeeded())
	{
		_enemyStatTable = EnemyStatTableFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> WaveDesignTableFinder(TEXT("/Script/Engine.DataTable'/Game/Data/DT_WaveDesigns.DT_WaveDesigns'"));
	if (WaveDesignTableFinder.Succeeded())
	{
		_waveDesignTable = WaveDesignTableFinder.Object;
	}

	ConfigureShipData();
	ConfigureEnemyStatData();
	ConfigureWaveDesigns();
	LoadDataTables();
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

	static ConstructorHelpers::FObjectFinder<USoundBase> FirstKillSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/FirstKill.FirstKill'"));
	if (FirstKillSoundFinder.Succeeded())
	{
		_firstKillSound = FirstKillSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> DoubleKillSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/DoubleKill.DoubleKill'"));
	if (DoubleKillSoundFinder.Succeeded())
	{
		_doubleKillSound = DoubleKillSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> TripleKillSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/TripleKill.TripleKill'"));
	if (TripleKillSoundFinder.Succeeded())
	{
		_tripleKillSound = TripleKillSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> QuadraKillSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/QuadraKill.QuadraKill'"));
	if (QuadraKillSoundFinder.Succeeded())
	{
		_quadraKillSound = QuadraKillSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> PentaKillSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/PentaKill.PentaKill'"));
	if (PentaKillSoundFinder.Succeeded())
	{
		_pentaKillSound = PentaKillSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> RampageSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/Rampage.Rampage'"));
	if (RampageSoundFinder.Succeeded())
	{
		_rampageSound = RampageSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> GotchaSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/Gotcha.Gotcha'"));
	if (GotchaSoundFinder.Succeeded())
	{
		_gotchaSound = GotchaSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> BossCutSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/BossCut.BossCut'"));
	if (BossCutSoundFinder.Succeeded())
	{
		_bossCutSound = BossCutSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> BossClearSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/BossClear.BossClear'"));
	if (BossClearSoundFinder.Succeeded())
	{
		_bossClearSound = BossClearSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> ClickSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/Click.Click'"));
	if (ClickSoundFinder.Succeeded())
	{
		_clickSound = ClickSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> GameStartSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/GameStart.GameStart'"));
	if (GameStartSoundFinder.Succeeded())
	{
		_gameStartSound = GameStartSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> UltimateReadySoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/UltimateReady.UltimateReady'"));
	if (UltimateReadySoundFinder.Succeeded())
	{
		_ultimateReadySound = UltimateReadySoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> QuitSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/PlayerQuit.PlayerQuit'"));
	if (QuitSoundFinder.Succeeded())
	{
		_quitSound = QuitSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> SelectCharacterSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/SelectCharacter.SelectCharacter'"));
	if (SelectCharacterSoundFinder.Succeeded())
	{
		_selectCharacterSound = SelectCharacterSoundFinder.Object;
	}
}

void AShootGameMode::BeginPlay()
{
	Super::BeginPlay();

	_masterSoundMix = NewObject<USoundMix>(this, TEXT("RuntimeMasterSoundMix"));
	_masterSoundClass = GetDefault<UAudioSettings>()->GetDefaultSoundClass();
	ApplyMasterVolume();

	_voiceAudioComponent = NewObject<UAudioComponent>(this, TEXT("RuntimeVoiceAudioComponent"));
	if (_voiceAudioComponent)
	{
		_voiceAudioComponent->bAutoActivate = false;
		_voiceAudioComponent->bAllowSpatialization = false;
		_voiceAudioComponent->RegisterComponent();
		_voiceAudioComponent->OnAudioFinished.AddDynamic(this, &AShootGameMode::HandleVoiceFinished);
	}

	_calloutAudioComponent = NewObject<UAudioComponent>(this, TEXT("RuntimeCalloutAudioComponent"));
	if (_calloutAudioComponent)
	{
		_calloutAudioComponent->bAutoActivate = false;
		_calloutAudioComponent->bAllowSpatialization = false;
		_calloutAudioComponent->RegisterComponent();
		_calloutAudioComponent->OnAudioFinished.AddDynamic(this, &AShootGameMode::HandleCalloutFinished);
	}

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

	PlayMenuClickSound();
	_gameState = EShootGameState::Lobby;
	UpdateInputMode();
}

void AShootGameMode::OpenDashboard()
{
	if (_gameState == EShootGameState::Playing)
	{
		return;
	}

	PlayMenuClickSound();
	_gameState = EShootGameState::Dashboard;
	UpdateInputMode();
}

void AShootGameMode::OpenShipSelect()
{
	if (_gameState == EShootGameState::Playing)
	{
		return;
	}

	PlayMenuClickSound();
	_gameState = EShootGameState::ShipSelect;
	ApplySelectedShipToPlayer();
	UpdateInputMode();
	PlayVoiceSound(_selectCharacterSound);
}

void AShootGameMode::SelectFalcon()
{
	if (_gameState != EShootGameState::ShipSelect)
	{
		return;
	}

	PlayMenuClickSound();
	_selectedShipData = _falconData;
	ApplySelectedShipToPlayer();
}

void AShootGameMode::SelectTitan()
{
	if (_gameState != EShootGameState::ShipSelect)
	{
		return;
	}

	PlayMenuClickSound();
	_selectedShipData = _titanData;
	ApplySelectedShipToPlayer();
}

void AShootGameMode::StartSelectedGame()
{
	if (_gameState != EShootGameState::ShipSelect)
	{
		return;
	}

	PlayMenuClickSound();
	_gameState = EShootGameState::Playing;
	UGameplayStatics::SetGamePaused(this, false);
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

	if (_gameStartSound)
	{
		PlayVoiceSound(_gameStartSound);
	}
	else if (_launchSound)
	{
		PlayVoiceSound(_launchSound);
	}
}

void AShootGameMode::RestartGame()
{
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

void AShootGameMode::QuitGame()
{
	if (_quitSound)
	{
		PlayVoiceSound(_quitSound);
		const float QuitDelay = GetVoiceQueueDelay(0.05f);
		FTimerHandle QuitTimerHandle;
		GetWorldTimerManager().SetTimer(QuitTimerHandle, this, &AShootGameMode::QuitGameNow, QuitDelay, false);
		return;
	}

	QuitGameNow();
}

void AShootGameMode::TogglePauseMenu()
{
	if (_gameState == EShootGameState::Playing)
	{
		_gameState = EShootGameState::PauseMenu;
		UGameplayStatics::SetGamePaused(this, true);
		UpdateInputMode();
		return;
	}

	if (_gameState == EShootGameState::PauseMenu)
	{
		ClosePauseMenu();
	}
}

void AShootGameMode::ClosePauseMenu()
{
	if (_gameState != EShootGameState::PauseMenu)
	{
		return;
	}

	PlayMenuClickSound();
	_gameState = EShootGameState::Playing;
	UGameplayStatics::SetGamePaused(this, false);
	UpdateInputMode();
}

void AShootGameMode::IncreaseMasterVolume()
{
	PlayMenuClickSound();
	_masterVolume = FMath::Clamp(_masterVolume + 0.1f, 0.0f, 1.0f);
	ApplyMasterVolume();
}

void AShootGameMode::DecreaseMasterVolume()
{
	PlayMenuClickSound();
	_masterVolume = FMath::Clamp(_masterVolume - 0.1f, 0.0f, 1.0f);
	ApplyMasterVolume();
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

	PlayKillCallout(IsBoss);
	if (!IsBoss && _killsThisWave == KillsRequiredForUltimate && Player && !Player->IsUltimateReady())
	{
		_pendingUltimateReadyWave = _currentWave;
		const float ReadyDelay = FMath::Max(GetVoiceQueueDelay(0.0f), GetCalloutQueueDelay(0.0f));
		GetWorldTimerManager().ClearTimer(_ultimateReadyTimerHandle);
		GetWorldTimerManager().SetTimer(_ultimateReadyTimerHandle, this, &AShootGameMode::EnableUltimateForCurrentWave, ReadyDelay, false);
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
	UGameplayStatics::SetGamePaused(this, false);
	_didPlayerWin = DidWin;
	GetWorldTimerManager().ClearTimer(_enemySpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(_waveTimerHandle);
	GetWorldTimerManager().ClearTimer(_ultimateReadyTimerHandle);
	SaveLeaderboardEntry(DidWin);
	UpdateInputMode();

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
	return IsValid(_bossEnemy) && (_gameState == EShootGameState::Playing || _gameState == EShootGameState::PauseMenu);
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
	if ((_gameState != EShootGameState::Playing && _gameState != EShootGameState::PauseMenu) || IsBossActive())
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

float AShootGameMode::GetMasterVolume() const
{
	return _masterVolume;
}

bool AShootGameMode::IsPauseMenuOpen() const
{
	return _gameState == EShootGameState::PauseMenu;
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
	_falconData.UltimateDamage = 210.0f;
	_falconData.FireInterval = 0.08f;
	_falconData.ShipColor = FLinearColor(0.0f, 0.55f, 1.0f, 1.0f);

	_titanData.ShipType = EPlayerShipType::Titan;
	_titanData.DisplayName = TEXT("Titan");
	_titanData.CruiseSpeed = 500.0f;
	_titanData.StrafeSpeed = 760.0f;
	_titanData.VerticalSpeed = 650.0f;
	_titanData.MaxHealth = 170.0f;
	_titanData.BulletDamage = 34.0f;
	_titanData.UltimateDamage = 260.0f;
	_titanData.FireInterval = 0.18f;
	_titanData.ShipColor = FLinearColor(1.0f, 0.22f, 0.04f, 1.0f);
}

void AShootGameMode::ConfigureEnemyStatData()
{
	_basicEnemyData.DisplayName = TEXT("Basic Drone");
	_basicEnemyData.MoveSpeed = 560.0f;
	_basicEnemyData.MaxHealth = 35.0f;
	_basicEnemyData.ContactDamage = 14.0f;
	_basicEnemyData.ScoreValue = 100;
	_basicEnemyData.MeshScale = FVector(1.1f, 1.1f, 0.7f);
	_basicEnemyData.Color = FLinearColor(0.55f, 0.9f, 1.0f, 1.0f);

	_fastEnemyData.DisplayName = TEXT("Red Fang");
	_fastEnemyData.MoveSpeed = 795.0f;
	_fastEnemyData.MaxHealth = 28.0f;
	_fastEnemyData.ContactDamage = 16.0f;
	_fastEnemyData.ScoreValue = 170;
	_fastEnemyData.MeshScale = FVector(0.86f, 0.86f, 0.5f);
	_fastEnemyData.Color = FLinearColor(1.0f, 0.08f, 0.1f, 1.0f);

	_tankEnemyData.DisplayName = TEXT("Iron Guard");
	_tankEnemyData.MoveSpeed = 405.0f;
	_tankEnemyData.MaxHealth = 75.0f;
	_tankEnemyData.ContactDamage = 20.0f;
	_tankEnemyData.ScoreValue = 260;
	_tankEnemyData.MeshScale = FVector(1.65f, 1.65f, 1.05f);
	_tankEnemyData.Color = FLinearColor(0.95f, 0.72f, 0.12f, 1.0f);

	_bossEnemyData.DisplayName = TEXT("Command Carrier");
	_bossEnemyData.MoveSpeed = 105.0f;
	_bossEnemyData.MaxHealth = 760.0f;
	_bossEnemyData.ContactDamage = 55.0f;
	_bossEnemyData.ScoreValue = 2500;
	_bossEnemyData.MeshScale = FVector(3.2f, 2.6f, 1.7f);
	_bossEnemyData.Color = FLinearColor(0.95f, 0.16f, 0.1f, 1.0f);
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

void AShootGameMode::LoadDataTables()
{
	if (_playerStatTable && _playerStatTable->GetRowStruct() == FPlayerShipData::StaticStruct())
	{
		if (const FPlayerShipData* FalconRow = _playerStatTable->FindRow<FPlayerShipData>(TEXT("Falcon"), TEXT("Load player stat table")))
		{
			_falconData = *FalconRow;
		}

		if (const FPlayerShipData* TitanRow = _playerStatTable->FindRow<FPlayerShipData>(TEXT("Titan"), TEXT("Load player stat table")))
		{
			_titanData = *TitanRow;
		}
	}

	if (_enemyStatTable && _enemyStatTable->GetRowStruct() == FEnemyStatData::StaticStruct())
	{
		if (const FEnemyStatData* BasicRow = _enemyStatTable->FindRow<FEnemyStatData>(TEXT("Basic"), TEXT("Load enemy stat table")))
		{
			_basicEnemyData = *BasicRow;
		}

		if (const FEnemyStatData* FastRow = _enemyStatTable->FindRow<FEnemyStatData>(TEXT("Fast"), TEXT("Load enemy stat table")))
		{
			_fastEnemyData = *FastRow;
		}

		if (const FEnemyStatData* TankRow = _enemyStatTable->FindRow<FEnemyStatData>(TEXT("Tank"), TEXT("Load enemy stat table")))
		{
			_tankEnemyData = *TankRow;
		}

		if (const FEnemyStatData* BossRow = _enemyStatTable->FindRow<FEnemyStatData>(TEXT("Boss"), TEXT("Load enemy stat table")))
		{
			_bossEnemyData = *BossRow;
		}
	}

	if (_waveDesignTable && _waveDesignTable->GetRowStruct() == FWaveDesign::StaticStruct())
	{
		TArray<FWaveDesign*> WaveRows;
		_waveDesignTable->GetAllRows<FWaveDesign>(TEXT("Load wave design table"), WaveRows);
		if (WaveRows.Num() > 0)
		{
			_waveDesigns.Empty();
			for (const FWaveDesign* WaveRow : WaveRows)
			{
				if (WaveRow)
				{
					_waveDesigns.Add(*WaveRow);
				}
			}

			_waveDesigns.Sort([](const FWaveDesign& A, const FWaveDesign& B)
			{
				return A.WaveNumber < B.WaveNumber;
			});

			_finalBossWave = _waveDesigns.Last().WaveNumber;
		}
	}
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

	if (_gameState == EShootGameState::PauseMenu)
	{
		AShootPlayerController* ShootPlayerController = Cast<AShootPlayerController>(PlayerController);
		if (ShootPlayerController)
		{
			ShootPlayerController->ApplyPauseInputMode();
			return;
		}

		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableMouseOverEvents = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
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
	_killsThisWave = 0;
	_pendingUltimateReadyWave = 0;
	GetWorldTimerManager().ClearTimer(_ultimateReadyTimerHandle);
	if (ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Player->ResetUltimateForWave();
	}

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
	const float SpeedScale = WaveDesign.BasicEnemySpeed > 0.0f ? WaveDesign.BasicEnemySpeed / 560.0f : 1.0f;
	const float HealthScale = WaveDesign.BasicEnemyHealth > 0.0f ? WaveDesign.BasicEnemyHealth / 35.0f : 1.0f;
	const float DamageScale = WaveDesign.BasicEnemyDamage > 0.0f ? WaveDesign.BasicEnemyDamage / 14.0f : 1.0f;
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
				_tankEnemyData.MoveSpeed * SpeedScale,
				_tankEnemyData.MaxHealth * HealthScale,
				_tankEnemyData.ContactDamage * DamageScale,
				_tankEnemyData.ScoreValue,
				_tankEnemyData.MeshScale,
				_tankEnemyData.Color);
			continue;
		}

		if (Roll < WaveDesign.TankEnemyChance + WaveDesign.FastEnemyChance)
		{
			Enemy->InitializeEnemy(
				_fastEnemyData.MoveSpeed * SpeedScale,
				_fastEnemyData.MaxHealth * HealthScale,
				_fastEnemyData.ContactDamage * DamageScale,
				_fastEnemyData.ScoreValue,
				_fastEnemyData.MeshScale,
				_fastEnemyData.Color);
			continue;
		}

		Enemy->InitializeEnemy(
			_basicEnemyData.MoveSpeed * SpeedScale,
			_basicEnemyData.MaxHealth * HealthScale,
			_basicEnemyData.ContactDamage * DamageScale,
			_basicEnemyData.ScoreValue,
			_basicEnemyData.MeshScale,
			_basicEnemyData.Color);
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
	_killsThisWave = 0;
	_pendingUltimateReadyWave = 0;
	GetWorldTimerManager().ClearTimer(_ultimateReadyTimerHandle);
	if (ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Player->ResetUltimateForWave();
	}

	const FVector SpawnLocation = GetSpawnLocation(1850.0f);
	const FRotator SpawnRotation(0.0f, 180.0f, 0.0f);
	_bossEnemy = GetWorld()->SpawnActor<ABossEnemy>(_bossClass, SpawnLocation, SpawnRotation);
	if (_bossEnemy)
	{
		_bossEnemy->InitializeEnemy(
			_bossEnemyData.MoveSpeed,
			_bossEnemyData.MaxHealth,
			_bossEnemyData.ContactDamage,
			_bossEnemyData.ScoreValue,
			_bossEnemyData.MeshScale,
			_bossEnemyData.Color);
	}

	if (_bossWarningSound)
	{
		PlayVoiceSound(_bossWarningSound);
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

void AShootGameMode::PlayKillCallout(bool IsBoss)
{
	if (IsBoss)
	{
		PlayCalloutSound(_bossCutSound);
		PlayCalloutSound(_bossClearSound, 2.0f);
		return;
	}

	_killsThisWave++;

	USoundBase* CalloutSound = nullptr;
	switch (_killsThisWave)
	{
	case 1:
		CalloutSound = _firstKillSound;
		break;
	case 2:
		CalloutSound = _doubleKillSound;
		break;
	case 3:
		CalloutSound = _tripleKillSound;
		break;
	case 4:
		CalloutSound = _quadraKillSound;
		break;
	case 5:
		CalloutSound = _pentaKillSound;
		break;
	case 6:
		CalloutSound = _rampageSound;
		break;
	default:
		CalloutSound = _gotchaSound;
		break;
	}

	if (CalloutSound)
	{
		PlayCalloutSound(CalloutSound);
	}
}

void AShootGameMode::PlayMenuClickSound()
{
	if (_clickSound && _gameState != EShootGameState::Playing)
	{
		PlayVoiceSound(_clickSound);
	}
}

void AShootGameMode::PlayVoiceSound(USoundBase* Sound, float ExtraDelay)
{
	if (!Sound || !GetWorld())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (_voiceSoundQueue.Num() == 0 && !GetWorldTimerManager().IsTimerActive(_voiceDelayTimerHandle) && (!_voiceAudioComponent || !_voiceAudioComponent->IsPlaying()))
	{
		_nextVoiceSoundTime = Now;
	}

	const float StartTime = FMath::Max(Now, _nextVoiceSoundTime) + FMath::Max(0.0f, ExtraDelay);
	const float Duration = FMath::Max(0.05f, Sound->GetDuration());
	_nextVoiceSoundTime = StartTime + Duration;

	_voiceSoundQueue.Add(Sound);
	_voiceDelayQueue.Add(FMath::Max(0.0f, ExtraDelay));
	_voiceImportantQueue.Add(false);
	PlayNextVoiceSound();
}

void AShootGameMode::PlayImportantVoiceSound(USoundBase* Sound)
{
	if (!Sound || !GetWorld())
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(_voiceDelayTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(_voiceDelayTimerHandle);
	}

	_voiceSoundQueue.Insert(Sound, 0);
	_voiceDelayQueue.Insert(0.0f, 0);
	_voiceImportantQueue.Insert(true, 0);
	PlayNextVoiceSound();
}

void AShootGameMode::PlayCalloutSound(USoundBase* Sound, float ExtraDelay)
{
	if (!Sound || !GetWorld())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (_calloutSoundQueue.Num() == 0 && !GetWorldTimerManager().IsTimerActive(_calloutDelayTimerHandle) && (!_calloutAudioComponent || !_calloutAudioComponent->IsPlaying()))
	{
		_nextCalloutSoundTime = Now;
	}

	const float StartTime = FMath::Max(Now, _nextCalloutSoundTime) + FMath::Max(0.0f, ExtraDelay);
	const float Duration = FMath::Max(0.05f, Sound->GetDuration());
	_nextCalloutSoundTime = StartTime + Duration;

	_calloutSoundQueue.Add(Sound);
	_calloutDelayQueue.Add(FMath::Max(0.0f, ExtraDelay));
	PlayNextCalloutSound();
}

void AShootGameMode::PlayNextVoiceSound()
{
	if (!GetWorld() || _voiceSoundQueue.Num() == 0)
	{
		return;
	}

	if (_voiceAudioComponent && _voiceAudioComponent->IsPlaying())
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(_voiceDelayTimerHandle))
	{
		return;
	}

	const bool IsImportantVoice = _voiceImportantQueue.IsValidIndex(0) && _voiceImportantQueue[0];
	if (IsImportantVoice && HasBlockingCalloutSound())
	{
		return;
	}

	const float Delay = _voiceDelayQueue.IsValidIndex(0) ? _voiceDelayQueue[0] : 0.0f;
	if (Delay > 0.0f)
	{
		_voiceDelayQueue[0] = 0.0f;
		GetWorldTimerManager().SetTimer(_voiceDelayTimerHandle, this, &AShootGameMode::PlayNextVoiceSound, Delay, false);
		return;
	}

	USoundBase* Sound = _voiceSoundQueue[0];
	_voiceSoundQueue.RemoveAt(0);
	if (_voiceDelayQueue.Num() > 0)
	{
		_voiceDelayQueue.RemoveAt(0);
	}
	if (_voiceImportantQueue.Num() > 0)
	{
		_voiceImportantQueue.RemoveAt(0);
	}

	if (!Sound)
	{
		PlayNextVoiceSound();
		return;
	}

	if (_voiceAudioComponent)
	{
		_voiceAudioComponent->SetSound(Sound);
		_voiceAudioComponent->SetVolumeMultiplier(1.0f);
		_voiceAudioComponent->Play(0.0f);
		return;
	}

	UGameplayStatics::PlaySound2D(this, Sound, 1.0f);
	PlayNextVoiceSound();
}

void AShootGameMode::PlayNextCalloutSound()
{
	if (!GetWorld() || _calloutSoundQueue.Num() == 0)
	{
		return;
	}

	if (_calloutAudioComponent && _calloutAudioComponent->IsPlaying())
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(_calloutDelayTimerHandle))
	{
		return;
	}

	const float Delay = _calloutDelayQueue.IsValidIndex(0) ? _calloutDelayQueue[0] : 0.0f;
	if (Delay > 0.0f)
	{
		_calloutDelayQueue[0] = 0.0f;
		GetWorldTimerManager().SetTimer(_calloutDelayTimerHandle, this, &AShootGameMode::PlayNextCalloutSound, Delay, false);
		return;
	}

	USoundBase* Sound = _calloutSoundQueue[0];
	_calloutSoundQueue.RemoveAt(0);
	if (_calloutDelayQueue.Num() > 0)
	{
		_calloutDelayQueue.RemoveAt(0);
	}

	if (!Sound)
	{
		PlayNextCalloutSound();
		return;
	}

	if (_calloutAudioComponent)
	{
		_calloutAudioComponent->SetSound(Sound);
		_calloutAudioComponent->SetVolumeMultiplier(1.0f);
		_calloutAudioComponent->Play(0.0f);
		return;
	}

	UGameplayStatics::PlaySound2D(this, Sound, 1.0f);
	PlayNextCalloutSound();
}

float AShootGameMode::GetVoiceQueueDelay(float ExtraDelay) const
{
	if (!GetWorld())
	{
		return FMath::Max(0.0f, ExtraDelay);
	}

	const float Now = GetWorld()->GetTimeSeconds();
	return FMath::Max(0.0f, _nextVoiceSoundTime - Now) + FMath::Max(0.0f, ExtraDelay);
}

float AShootGameMode::GetCalloutQueueDelay(float ExtraDelay) const
{
	if (!GetWorld())
	{
		return FMath::Max(0.0f, ExtraDelay);
	}

	const float Now = GetWorld()->GetTimeSeconds();
	return FMath::Max(0.0f, _nextCalloutSoundTime - Now) + FMath::Max(0.0f, ExtraDelay);
}

bool AShootGameMode::HasBlockingCalloutSound() const
{
	const bool IsCalloutPlaying = _calloutAudioComponent && _calloutAudioComponent->IsPlaying();
	const bool IsCalloutDelayed = GetWorld() && GetWorldTimerManager().IsTimerActive(_calloutDelayTimerHandle);
	return IsCalloutPlaying || IsCalloutDelayed;
}

void AShootGameMode::EnableUltimateForCurrentWave()
{
	if (_gameState != EShootGameState::Playing || _pendingUltimateReadyWave != _currentWave)
	{
		return;
	}

	ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player || Player->IsUltimateReady())
	{
		return;
	}

	Player->SetUltimateReady(true);
	PlayImportantVoiceSound(_ultimateReadySound);
}

void AShootGameMode::HandleVoiceFinished()
{
	PlayNextVoiceSound();
}

void AShootGameMode::HandleCalloutFinished()
{
	PlayNextCalloutSound();
	PlayNextVoiceSound();
}

void AShootGameMode::QuitGameNow()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, true);
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

void AShootGameMode::ApplyMasterVolume()
{
	if (!_masterSoundMix || !_masterSoundClass)
	{
		return;
	}

	UGameplayStatics::SetSoundMixClassOverride(this, _masterSoundMix, _masterSoundClass, _masterVolume, 1.0f, 0.05f, true);
	UGameplayStatics::PushSoundMixModifier(this, _masterSoundMix);
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
