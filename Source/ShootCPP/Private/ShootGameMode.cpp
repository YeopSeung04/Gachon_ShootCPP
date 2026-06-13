#include "ShootGameMode.h"

#include "BossEnemy.h"
#include "CPlayer.h"
#include "Components/AudioComponent.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LeaderboardSaveGame.h"
#include "ShootGameInstance.h"
#include "ShootHUD.h"
#include "ShootPlayerController.h"
#include "Sound/AudioSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundBase.h"
#include "SpaceArena.h"
#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// 레벨 이름을 상수로 모아 두면 오타로 인한 이동 실패를 줄일 수 있다.
	const FName LobbyMapName(TEXT("LobbyMap"));
	const FName CharacterSelectMapName(TEXT("CharacterSelectMap"));
	const FName GameplayMapName(TEXT("GameplayMap"));
	const FName LegacyShootingMapName(TEXT("ShootingMap"));
	const FName LobbyMapPackageName(TEXT("/Game/Maps/LobbyMap"));
	const FName CharacterSelectMapPackageName(TEXT("/Game/Maps/CharacterSelectMap"));
	const FName GameplayMapPackageName(TEXT("/Game/Maps/GameplayMap"));
}

AShootGameMode::AShootGameMode()
{
	// GameMode는 이 프로젝트의 중앙 관리자다.
	// 기본 Pawn/HUD/Controller와 적 클래스를 지정해 레벨마다 같은 규칙으로 게임이 시작되게 한다.
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
	// C++ 기본값을 먼저 만든 뒤 DataTable을 읽는다.
	// DataTable이 없거나 구조가 다르면 기본값으로도 게임이 실행된다.
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

	// 1킬
	static ConstructorHelpers::FObjectFinder<USoundBase> FirstKillSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/FirstKill.FirstKill'"));
	if (FirstKillSoundFinder.Succeeded())
	{
		_firstKillSound = FirstKillSoundFinder.Object;
	}

	// 2킬
	static ConstructorHelpers::FObjectFinder<USoundBase> DoubleKillSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/DoubleKill.DoubleKill'"));
	if (DoubleKillSoundFinder.Succeeded())
	{
		_doubleKillSound = DoubleKillSoundFinder.Object;
	}

	// 3
	static ConstructorHelpers::FObjectFinder<USoundBase> TripleKillSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/TripleKill.TripleKill'"));
	if (TripleKillSoundFinder.Succeeded())
	{
		_tripleKillSound = TripleKillSoundFinder.Object;
	}


	//4
	static ConstructorHelpers::FObjectFinder<USoundBase> QuadraKillSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/QuadraKill.QuadraKill'"));
	if (QuadraKillSoundFinder.Succeeded())
	{
		_quadraKillSound = QuadraKillSoundFinder.Object;
	}

	//5
	static ConstructorHelpers::FObjectFinder<USoundBase> PentaKillSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/PentaKill.PentaKill'"));
	if (PentaKillSoundFinder.Succeeded())
	{
		_pentaKillSound = PentaKillSoundFinder.Object;
	}

	// 미쳐 날뛴다
	static ConstructorHelpers::FObjectFinder<USoundBase> RampageSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/Rampage.Rampage'"));
	if (RampageSoundFinder.Succeeded())
	{
		_rampageSound = RampageSoundFinder.Object;
	}

	// 7킬 이후 나오는 소리
	static ConstructorHelpers::FObjectFinder<USoundBase> GotchaSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/Gotcha.Gotcha'"));
	if (GotchaSoundFinder.Succeeded())
	{
		_gotchaSound = GotchaSoundFinder.Object;
	}

	// 보스 컷
	static ConstructorHelpers::FObjectFinder<USoundBase> BossCutSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/BossCut.BossCut'"));
	if (BossCutSoundFinder.Succeeded())
	{
		_bossCutSound = BossCutSoundFinder.Object;
	}

	// 보스 클리어
	static ConstructorHelpers::FObjectFinder<USoundBase> BossClearSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/BossClear.BossClear'"));
	if (BossClearSoundFinder.Succeeded())
	{
		_bossClearSound = BossClearSoundFinder.Object;
	}

	// 클릭 소리
	static ConstructorHelpers::FObjectFinder<USoundBase> ClickSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/Click.Click'"));
	if (ClickSoundFinder.Succeeded())
	{
		_clickSound = ClickSoundFinder.Object;
	}

	// 게임 시작
	static ConstructorHelpers::FObjectFinder<USoundBase> GameStartSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/GameStart.GameStart'"));
	if (GameStartSoundFinder.Succeeded())
	{
		_gameStartSound = GameStartSoundFinder.Object;
	}

	// 궁 충전
	static ConstructorHelpers::FObjectFinder<USoundBase> UltimateReadySoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/UltimateReady.UltimateReady'"));
	if (UltimateReadySoundFinder.Succeeded())
	{
		_ultimateReadySound = UltimateReadySoundFinder.Object;
	}

	// 게임 종료
	static ConstructorHelpers::FObjectFinder<USoundBase> QuitSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/PlayerQuit.PlayerQuit'"));
	if (QuitSoundFinder.Succeeded())
	{
		_quitSound = QuitSoundFinder.Object;
	}

	// 캐릭터 선택
	static ConstructorHelpers::FObjectFinder<USoundBase> SelectCharacterSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/SelectCharacter.SelectCharacter'"));
	if (SelectCharacterSoundFinder.Succeeded())
	{
		_selectCharacterSound = SelectCharacterSoundFinder.Object;
	}
}

void AShootGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 런타임 SoundMix를 만들어 일시정지 메뉴의 볼륨 조절이 전체 사운드에 적용되게 한다.
	_masterSoundMix = NewObject<USoundMix>(this, TEXT("RuntimeMasterSoundMix"));
	_masterSoundClass = GetDefault<UAudioSettings>()->GetDefaultSoundClass();
	ApplyMasterVolume();

	// 음성 안내와 킬 콜아웃은 별도 AudioComponent로 재생한다.
	// 끝났다는 이벤트를 받아 큐의 다음 사운드를 이어서 재생할 수 있다.
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
	LoadPersistedShipSelection();
	// 현재 열린 맵 이름으로 Lobby/ShipSelect/Playing 중 어느 상태로 시작할지 결정한다.
	InitializeStateForCurrentMap();
	EnsureSpaceArena();
	ApplySelectedShipToPlayer();
	UpdateInputMode();

	if (_gameState == EShootGameState::Playing)
	{
		StartGameplaySession();
		return;
	}

	if (_gameState == EShootGameState::ShipSelect)
	{
		PlayVoiceSound(_selectCharacterSound);
	}
}

void AShootGameMode::OpenLobby()
{
	if (_gameState == EShootGameState::Playing)
	{
		// 전투 중에는 실수로 로비로 빠져나가지 못하게 막는다.
		return;
	}

	PlayMenuClickSound();
	if (!IsCurrentMap(LobbyMapName) && !IsCurrentMap(LegacyShootingMapName))
	{
		// 메뉴 상태만 바꾸는 것이 아니라 필요한 레벨이 다르면 실제 맵 이동을 수행한다.
		QueueLevelTransition(LobbyMapName);
		return;
	}

	_gameState = EShootGameState::Lobby;
	// 상태가 메뉴로 바뀌었으므로 마우스 커서와 UI 입력을 켠다.
	UpdateInputMode();
}

void AShootGameMode::OpenDashboard()
{
	if (_gameState == EShootGameState::Playing)
	{
		// 전투 중에는 기록 화면으로 이동하지 않는다.
		return;
	}

	PlayMenuClickSound();
	if (!IsCurrentMap(LobbyMapName) && !IsCurrentMap(LegacyShootingMapName))
	{
		QueueLevelTransition(LobbyMapName);
		return;
	}

	_gameState = EShootGameState::Dashboard;
	// Dashboard는 메뉴 화면이므로 GameAndUI 입력 모드를 사용한다.
	UpdateInputMode();
}

void AShootGameMode::OpenShipSelect()
{
	if (_gameState == EShootGameState::Playing)
	{
		// 전투 도중 기체를 바꾸면 밸런스와 체력 상태가 꼬이므로 막는다.
		return;
	}

	PlayMenuClickSound();
	if (!IsCurrentMap(CharacterSelectMapName) && !IsCurrentMap(LegacyShootingMapName))
	{
		// 맵 이동 전에 현재 선택값을 GameInstance에 저장해 새 GameMode에서도 이어받게 한다.
		SavePersistedShipSelection();
		QueueLevelTransition(CharacterSelectMapName);
		return;
	}

	_gameState = EShootGameState::ShipSelect;
	// 선택 화면에서도 플레이어 기체 미리보기가 현재 선택값과 맞도록 적용한다.
	ApplySelectedShipToPlayer();
	UpdateInputMode();
}

void AShootGameMode::SelectFalcon()
{
	if (_gameState != EShootGameState::ShipSelect)
	{
		// 기체 선택 화면에서만 선택 입력을 허용한다.
		return;
	}

	PlayMenuClickSound();
	// 내부 선택 데이터를 Falcon으로 바꾸고, 맵 이동에 대비해 GameInstance에도 저장한다.
	_selectedShipData = _falconData;
	SavePersistedShipSelection();
	ApplySelectedShipToPlayer();
}

void AShootGameMode::SelectTitan()
{
	if (_gameState != EShootGameState::ShipSelect)
	{
		// 기체 선택 화면에서만 선택 입력을 허용한다.
		return;
	}

	PlayMenuClickSound();
	// 내부 선택 데이터를 Titan으로 바꾸고, 맵 이동에 대비해 GameInstance에도 저장한다.
	_selectedShipData = _titanData;
	SavePersistedShipSelection();
	ApplySelectedShipToPlayer();
}

void AShootGameMode::StartSelectedGame()
{
	if (_gameState != EShootGameState::ShipSelect)
	{
		// 기체 선택이 끝난 뒤에만 전투를 시작할 수 있다.
		return;
	}

	PlayMenuClickSound();
	// GameplayMap으로 넘어가도 선택한 기체가 유지되도록 먼저 저장한다.
	SavePersistedShipSelection();
	if (!IsCurrentMap(GameplayMapName) && !IsCurrentMap(LegacyShootingMapName))
	{
		QueueLevelTransition(GameplayMapName);
		return;
	}

	StartGameplaySession();
}

void AShootGameMode::StartGameplaySession()
{
	// 실제 전투 시작 지점이다. 이전 상태를 정리하고 플레이어/웨이브/입력을 새 판 기준으로 초기화한다.
	_gameState = EShootGameState::Playing;
	UGameplayStatics::SetGamePaused(this, false);
	_currentWave = 1;
	_didPlayerWin = false;
	_playStartTime = GetWorld()->GetTimeSeconds();

	ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (Player)
	{
		// 새 게임마다 플레이어를 시작 위치로 보내고, 이전 판의 입력/체력/점수를 초기화한다.
		Player->SetActorLocation(FVector(0.0f, 0.0f, 360.0f), false, nullptr, ETeleportType::TeleportPhysics);
		Player->SetActorRotation(FRotator::ZeroRotator);
		Player->ResetInputState();
		Player->ApplyShipData(_selectedShipData);
		Player->ResetForGameplayStart();
	}

	StartWave();
	// 전투 상태로 바뀌었으므로 마우스 커서를 숨기고 게임 입력만 받는다.
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
	// 현재 상태를 복잡하게 되돌리지 않고 GameplayMap을 다시 열어 완전히 새 판으로 시작한다.
	UGameplayStatics::OpenLevel(this, GameplayMapPackageName, false);
}

void AShootGameMode::QuitGame()
{
	if (_quitSound)
	{
		PlayVoiceSound(_quitSound);
		// 종료 음성이 재생될 시간을 확보한 뒤 실제 QuitGame을 호출한다.
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
		// Playing -> PauseMenu로 상태를 바꾸고 Unreal의 일시정지도 같이 건다.
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
	// PauseMenu -> Playing으로 복귀한다. 입력 모드도 다시 GameOnly로 바뀐다.
	_gameState = EShootGameState::Playing;
	UGameplayStatics::SetGamePaused(this, false);
	UpdateInputMode();
}

void AShootGameMode::IncreaseMasterVolume()
{
	PlayMenuClickSound();
	// 볼륨은 0~1 범위로 유지한다. 0.1씩 올려 Pause 메뉴에서 단계적으로 조절된다.
	_masterVolume = FMath::Clamp(_masterVolume + 0.1f, 0.0f, 1.0f);
	ApplyMasterVolume();
}

void AShootGameMode::DecreaseMasterVolume()
{
	PlayMenuClickSound();
	// 볼륨은 0 아래로 내려가지 않도록 Clamp한다.
	_masterVolume = FMath::Clamp(_masterVolume - 0.1f, 0.0f, 1.0f);
	ApplyMasterVolume();
}

void AShootGameMode::HandlePrimaryClick(float ScreenX, float ScreenY)
{
	// Canvas HUD fallback을 위한 마우스 좌표 처리다.
	// UMG 위젯이 없을 때도 같은 메뉴 기능을 사용할 수 있게 사각형 영역을 직접 판정한다.
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
		// Lobby 화면의 버튼 영역: Play, Dashboard, Exit 순서다.
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
		// 기체 카드 자체를 클릭 영역으로 사용한다.
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
		// 게임오버 화면에서는 재시작 또는 로비 복귀만 허용한다.
		if (IsInsideRect(ScreenX, ScreenY, CenterX - 255.0f, ViewportSize.Y - 108.0f, 230.0f, 46.0f))
		{
			RestartGame();
			return;
		}

		if (IsInsideRect(ScreenX, ScreenY, CenterX + 25.0f, ViewportSize.Y - 108.0f, 230.0f, 46.0f))
		{
			OpenLobby();
			return;
		}
	}
}

void AShootGameMode::RegisterEnemyKilled(int32 ScoreValue, bool IsBoss)
{
	// 모든 적 처치 결과는 이 함수로 모인다.
	// 점수, 콜아웃, 궁극기 충전, 보스 클리어 판정을 한 곳에서 처리한다.
	ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (Player)
	{
		// 점수는 적 자신이 아니라 GameMode를 통해 Player에 더해진다.
		Player->AddScore(ScoreValue);
	}

	PlayKillCallout(IsBoss);
	if (!IsBoss
		&& Player
		&& !GetCurrentWaveDesign().IsBossWave
		&& !Player->IsUltimateReady()
		&& _pendingUltimateChargeCount <= 0
		&& (_currentWave >= 4 || _ultimateChargesAwardedThisWave <= 0))
	{
		// 일반 적 처치 수를 세다가 기준치에 도달하면 궁극기 1회를 충전한다.
		_killsSinceLastUltimateCharge++;
		if (_killsSinceLastUltimateCharge >= KillsRequiredForUltimate)
		{
			_killsSinceLastUltimateCharge = 0;
			_ultimateChargesAwardedThisWave++;
			Player->AddUltimateCharge();
			PlayImportantVoiceSound(_ultimateReadySound);
		}
	}

	if (IsBoss)
	{
		// 보스 처치가 최종 승리 조건이다.
		EndGame(true);
	}
}

void AShootGameMode::NotifyPlayerUltimateUsed()
{
	if (_gameState != EShootGameState::Playing || GetCurrentWaveDesign().IsBossWave)
	{
		// 전투 중이 아니거나 보스전이면 일반 웨이브용 궁극기 충전 규칙을 적용하지 않는다.
		return;
	}

	if (_currentWave >= 4)
	{
		// 후반 웨이브에서는 궁극기 사용 후 다시 처치 수를 쌓아 재충전할 수 있게 카운터를 초기화한다.
		_killsSinceLastUltimateCharge = 0;
		_ultimateChargesAwardedThisWave = 0;
		_pendingUltimateChargeCount = 0;
		_pendingUltimateReadyWave = 0;
		GetWorldTimerManager().ClearTimer(_ultimateReadyTimerHandle);
	}
}

void AShootGameMode::EndGame(bool DidWin)
{
	if (_gameState == EShootGameState::GameOver)
	{
		return;
	}

	// 게임 종료 시 스폰/웨이브/궁극기 타이머를 멈추고 리더보드에 결과를 저장한다.
	_gameState = EShootGameState::GameOver;
	UGameplayStatics::SetGamePaused(this, false);
	_didPlayerWin = DidWin;
	GetWorldTimerManager().ClearTimer(_enemySpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(_waveTimerHandle);
	GetWorldTimerManager().ClearTimer(_ultimateReadyTimerHandle);
	_pendingUltimateChargeCount = 0;
	// 종료 직후 결과 화면에서 바로 리더보드가 보이도록 즉시 저장한다.
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
		// 전투가 아니거나 보스전이면 일반 웨이브 진행바를 꽉 찬 상태로 처리한다.
		return 1.0f;
	}

	// 현재 시간과 웨이브 시작 시간의 차이를 웨이브 전체 시간으로 나누어 0~1 비율을 만든다.
	const float Elapsed = GetWorld()->GetTimeSeconds() - _waveStartTime;
	return FMath::Clamp(Elapsed / _waveDuration, 0.0f, 1.0f);
}

float AShootGameMode::GetSurvivedTime() const
{
	if (_playStartTime <= 0.0f)
	{
		// 전투가 아직 시작되지 않았으면 생존 시간을 0으로 본다.
		return 0.0f;
	}

	// 전투 시작 시각부터 현재 월드 시간까지의 차이가 생존 시간이다.
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
	// DataTable 로드 실패에 대비한 C++ 기본 기체 밸런스다.
	// Falcon: 속도/연사, Titan: 체력/단발 화력으로 역할을 나눴다.
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
	// 적 타입별 기본 밸런스다. 웨이브 데이터에서 배율을 곱해 난이도가 증가한다.
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
	_bossEnemyData.MoveSpeed = 260.0f;
	_bossEnemyData.MaxHealth = 760.0f;
	_bossEnemyData.ContactDamage = 72.0f;
	_bossEnemyData.ScoreValue = 2500;
	_bossEnemyData.MeshScale = FVector(3.2f, 2.6f, 1.7f);
	_bossEnemyData.Color = FLinearColor(0.95f, 0.16f, 0.1f, 1.0f);
}

void AShootGameMode::ConfigureWaveDesigns()
{
	// 웨이브 기본 설계다.
	// 1~4웨이브는 일반 적 스폰, 5웨이브는 보스전으로 구성된다.
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
	Wave5.IsBossWave = true;
	_waveDesigns.Add(Wave5);
}

void AShootGameMode::LoadDataTables()
{
	// 에디터에서 만든 DataTable이 있으면 C++ 기본값 대신 테이블 값을 사용한다.
	// 구조체 타입까지 확인해 잘못된 테이블 연결로 인한 런타임 오류를 줄인다.
	if (_playerStatTable && _playerStatTable->GetRowStruct() == FPlayerShipData::StaticStruct())
	{
		if (const FPlayerShipData* FalconRow = _playerStatTable->FindRow<FPlayerShipData>(TEXT("Falcon"), TEXT("Load player stat table")))
		{
			// DataTable에 Falcon 행이 있으면 C++ 기본 Falcon 값을 교체한다.
			_falconData = *FalconRow;
		}

		if (const FPlayerShipData* TitanRow = _playerStatTable->FindRow<FPlayerShipData>(TEXT("Titan"), TEXT("Load player stat table")))
		{
			// DataTable에 Titan 행이 있으면 C++ 기본 Titan 값을 교체한다.
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
			// 테이블에 웨이브가 하나라도 있으면 기본 웨이브 배열을 지우고 테이블 기준으로 다시 구성한다.
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
				// DataTable 행 순서와 상관없이 WaveNumber 기준으로 진행되게 정렬한다.
				return A.WaveNumber < B.WaveNumber;
			});

			// 마지막 행의 WaveNumber를 최종 보스 웨이브 번호로 사용한다.
			_finalBossWave = _waveDesigns.Last().WaveNumber;
		}
	}
}

void AShootGameMode::LoadPersistedShipSelection()
{
	// 맵이 바뀌면 GameMode는 새로 생성되므로, 선택한 기체는 GameInstance에 임시 보관한다.
	const UShootGameInstance* ShootGameInstance = GetGameInstance<UShootGameInstance>();
	if (!ShootGameInstance)
	{
		return;
	}

	_selectedShipData = ShootGameInstance->GetSelectedShipType() == EPlayerShipType::Titan ? _titanData : _falconData;
}

void AShootGameMode::SavePersistedShipSelection()
{
	UShootGameInstance* ShootGameInstance = GetGameInstance<UShootGameInstance>();
	if (!ShootGameInstance)
	{
		return;
	}

	// GameInstance는 맵 이동 후에도 살아 있으므로 선택한 기체 타입을 여기에 보관한다.
	ShootGameInstance->SetSelectedShipType(_selectedShipData.ShipType);
}

void AShootGameMode::InitializeStateForCurrentMap()
{
	if (IsCurrentMap(CharacterSelectMapName))
	{
		// 캐릭터 선택 맵에서 시작했다면 바로 ShipSelect 상태로 둔다.
		_gameState = EShootGameState::ShipSelect;
		return;
	}

	if (IsCurrentMap(GameplayMapName))
	{
		// 게임플레이 맵에서 시작했다면 BeginPlay 이후 StartGameplaySession이 호출된다.
		_gameState = EShootGameState::Playing;
		return;
	}

	// 그 외 맵은 기본적으로 로비 화면으로 본다.
	_gameState = EShootGameState::Lobby;
}

FName AShootGameMode::GetCurrentMapName() const
{
	if (!GetWorld())
	{
		return NAME_None;
	}

	FString MapName = GetWorld()->GetMapName();
	// PIE 실행 시 맵 이름 앞에 붙는 임시 prefix를 제거해 실제 맵 이름만 비교한다.
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	return FName(*MapName);
}

bool AShootGameMode::IsCurrentMap(FName MapName) const
{
	return GetCurrentMapName() == MapName;
}

void AShootGameMode::QueueLevelTransition(FName MapName)
{
	if (!GetWorld())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(_levelTransitionTimerHandle);
	if (MapName == LobbyMapName)
	{
		// 요청받은 목적지에 맞는 실제 레벨 열기 함수를 호출한다.
		OpenLobbyLevel();
		return;
	}

	if (MapName == CharacterSelectMapName)
	{
		OpenCharacterSelectLevel();
		return;
	}

	if (MapName == GameplayMapName)
	{
		OpenGameplayLevel();
	}
}

void AShootGameMode::ApplySelectedShipToPlayer()
{
	ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (Player)
	{
		// 현재 선택된 기체 데이터를 실제 Pawn에 적용한다.
		Player->ApplyShipData(_selectedShipData);
	}
}

void AShootGameMode::EnsureSpaceArena()
{
	if (IsValid(_spaceArena))
	{
		return;
	}

	TArray<AActor*> ArenaActors;
	UGameplayStatics::GetAllActorsOfClass(this, ASpaceArena::StaticClass(), ArenaActors);
	if (ArenaActors.Num() > 0)
	{
		// 맵 안의 첫 번째 SpaceArena를 찾아 보관한다.
		_spaceArena = Cast<ASpaceArena>(ArenaActors[0]);
	}
}

void AShootGameMode::UpdateInputMode()
{
	// 현재 게임 상태에 맞춰 마우스 커서와 입력 모드를 전환한다.
	// 메뉴는 GameAndUI, 전투는 GameOnly, 일시정지는 GameAndUI를 사용한다.
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

	// 웨이브 시작 시 카운터를 초기화하고, 반복 스폰 타이머와 웨이브 종료 타이머를 새로 건다.
	_waveStartTime = GetWorld()->GetTimeSeconds();
	_killsThisWave = 0;
	_killsSinceLastUltimateCharge = 0;
	_ultimateChargesAwardedThisWave = 0;
	_pendingUltimateChargeCount = 0;
	_pendingUltimateReadyWave = 0;
	GetWorldTimerManager().ClearTimer(_ultimateReadyTimerHandle);

	GetWorldTimerManager().ClearTimer(_enemySpawnTimerHandle);
	// 첫 스폰은 0.2초 뒤에 빠르게 나오고, 이후에는 웨이브별 SpawnInterval마다 반복된다.
	GetWorldTimerManager().SetTimer(_enemySpawnTimerHandle, this, &AShootGameMode::SpawnEnemy, GetCurrentSpawnInterval(), true, 0.2f);

	GetWorldTimerManager().ClearTimer(_waveTimerHandle);
	// _waveDuration 시간이 지나면 다음 웨이브로 넘어간다.
	GetWorldTimerManager().SetTimer(_waveTimerHandle, this, &AShootGameMode::AdvanceWave, _waveDuration, false);
}

void AShootGameMode::AdvanceWave()
{
	if (_gameState != EShootGameState::Playing)
	{
		return;
	}

	_currentWave++;

	// 다음 웨이브가 보스 웨이브라면 일반 스폰을 멈추고 보스를 소환한다.
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
	// 웨이브 데이터는 기본 적 능력치에 곱해지는 배율 역할을 한다.
	const float SpeedScale = WaveDesign.BasicEnemySpeed > 0.0f ? WaveDesign.BasicEnemySpeed / 560.0f : 1.0f;
	const float HealthScale = WaveDesign.BasicEnemyHealth > 0.0f ? WaveDesign.BasicEnemyHealth / 35.0f : 1.0f;
	const float DamageScale = WaveDesign.BasicEnemyDamage > 0.0f ? WaveDesign.BasicEnemyDamage / 14.0f : 1.0f;
	for (int32 SpawnIndex = 0; SpawnIndex < WaveDesign.EnemiesPerSpawn; ++SpawnIndex)
	{
		// 여러 마리가 동시에 나올 때 X 위치를 조금씩 벌려 겹쳐 생성되는 것을 줄인다.
		FVector SpawnLocation = GetSpawnLocation(1550.0f + SpawnIndex * 170.0f);
		SpawnLocation.Y += FMath::RandRange(-180.0f, 180.0f);
		const FRotator SpawnRotation(0.0f, 180.0f, 0.0f);
		AEnemy* Enemy = GetWorld()->SpawnActor<AEnemy>(_enemyClass, SpawnLocation, SpawnRotation);
		if (!Enemy)
		{
			// Spawn 실패 시 이 회차만 건너뛰고 다음 적 생성을 계속 시도한다.
			continue;
		}

		const float Roll = FMath::FRand();
		// 하나의 난수로 Tank -> Fast -> Basic 순서의 확률 분기를 만든다.
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

	// 보스전 시작 전 일반 적과 웨이브 타이머를 정리해 전투 목표를 보스 하나로 집중시킨다.
	GetWorldTimerManager().ClearTimer(_enemySpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(_waveTimerHandle);
	ClearEnemies();
	_killsThisWave = 0;
	_killsSinceLastUltimateCharge = 0;
	_ultimateChargesAwardedThisWave = 0;
	_pendingUltimateChargeCount = 0;
	_pendingUltimateReadyWave = 0;
	GetWorldTimerManager().ClearTimer(_ultimateReadyTimerHandle);
	if (ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		// 보스전은 체력이 높기 때문에 시작 보상으로 궁극기 2회를 지급한다.
		Player->SetUltimateChargeCount(2);
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
			// 보스전 전환 시 일반 적만 정리하고 보스 참조는 건드리지 않는다.
			Enemy->Destroy();
		}
	}
}

void AShootGameMode::PlayKillCallout(bool IsBoss)
{
	// 처치 수에 따라 First/Double/Triple 같은 음성 콜아웃을 단계적으로 재생한다.
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
		// 6킬 이후부터는 쿨타임이 허용될 때 Gotcha 음성으로 반복 처리한다.
		PlayGotchaCallout();
		return;
	}

	if (CalloutSound)
	{
		PlayCalloutSound(CalloutSound);
	}
}

void AShootGameMode::PlayGotchaCallout()
{
	if (!_gotchaSound || !GetWorld())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now < _nextGotchaCalloutTime)
	{
		// 같은 음성이 너무 자주 반복되지 않게 시간 제한을 둔다.
		return;
	}

	const bool IsCalloutBusy = (_calloutAudioComponent && _calloutAudioComponent->IsPlaying())
		|| GetWorldTimerManager().IsTimerActive(_calloutDelayTimerHandle)
		|| _calloutSoundQueue.Num() > 0;
	if (IsCalloutBusy)
	{
		// 다른 콜아웃이 재생/대기 중이면 Gotcha를 억지로 끼워 넣지 않는다.
		return;
	}

	_nextGotchaCalloutTime = Now + GotchaCalloutCooldown;
	if (_calloutAudioComponent)
	{
		_calloutAudioComponent->SetSound(_gotchaSound);
		_calloutAudioComponent->SetVolumeMultiplier(1.0f);
		_calloutAudioComponent->Play(0.0f);
		return;
	}

	UGameplayStatics::PlaySound2D(this, _gotchaSound, 1.0f);
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
	// 재생 예약 시간을 계산해 사운드가 서로 덮어쓰지 않고 순서대로 나오도록 큐에 넣는다.
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

	// 궁극기 준비/발사처럼 중요한 안내는 큐 앞에 넣어 일반 안내보다 먼저 들리게 한다.
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
		// 콜아웃 큐가 비어 있으면 현재 시각을 기준으로 새 재생 일정을 시작한다.
		_nextCalloutSoundTime = Now;
	}

	const float StartTime = FMath::Max(Now, _nextCalloutSoundTime) + FMath::Max(0.0f, ExtraDelay);
	const float Duration = FMath::Max(0.05f, Sound->GetDuration());
	// 다음 콜아웃이 시작될 수 있는 시간을 현재 소리 길이만큼 뒤로 민다.
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
		// 이미 음성이 재생 중이면 끝났다는 이벤트가 올 때까지 기다린다.
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(_voiceDelayTimerHandle))
	{
		return;
	}

	const bool IsImportantVoice = _voiceImportantQueue.IsValidIndex(0) && _voiceImportantQueue[0];
	if (IsImportantVoice && HasBlockingCalloutSound())
	{
		// 킬 콜아웃이 재생 중이면 중요한 안내도 잠깐 기다려 음성이 겹치지 않게 한다.
		return;
	}

	const float Delay = _voiceDelayQueue.IsValidIndex(0) ? _voiceDelayQueue[0] : 0.0f;
	if (Delay > 0.0f)
	{
		// 예약 지연 시간이 남아 있으면 타이머로 다시 PlayNextVoiceSound를 호출한다.
		_voiceDelayQueue[0] = 0.0f;
		GetWorldTimerManager().SetTimer(_voiceDelayTimerHandle, this, &AShootGameMode::PlayNextVoiceSound, Delay, false);
		return;
	}

	USoundBase* Sound = _voiceSoundQueue[0];
	// 큐의 맨 앞 소리를 꺼내고, 같은 인덱스의 delay/important 정보도 같이 제거한다.
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
		// 현재 콜아웃이 끝나야 다음 콜아웃을 재생한다.
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
	// 콜아웃 큐의 맨 앞 소리를 꺼낸다.
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
	// 이미 예약된 음성이 끝나는 시간까지 기다린 뒤 ExtraDelay를 추가한다.
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
	if (_gameState != EShootGameState::Playing || _pendingUltimateReadyWave != _currentWave || _pendingUltimateChargeCount <= 0)
	{
		return;
	}

	ACPlayer* Player = Cast<ACPlayer>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player || !Player->CanChargeUltimateThisWave())
	{
		return;
	}

	Player->AddUltimateCharge();
	_pendingUltimateChargeCount--;
	PlayImportantVoiceSound(_ultimateReadySound);
	if (_pendingUltimateChargeCount > 0)
	{
		// 여러 충전이 동시에 예약된 경우 한 프레임에 몰아주지 않고 짧은 간격으로 지급한다.
		GetWorldTimerManager().SetTimer(_ultimateReadyTimerHandle, this, &AShootGameMode::EnableUltimateForCurrentWave, 0.2f, false);
	}
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

void AShootGameMode::OpenLobbyLevel()
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		// PlayerController가 있으면 ClientTravel로 이동해 플레이어 컨트롤 흐름을 유지한다.
		PlayerController->ClientTravel(LobbyMapPackageName.ToString(), TRAVEL_Absolute);
		return;
	}

	// Controller가 없는 예외 상황에서는 OpenLevel로 직접 맵을 연다.
	UGameplayStatics::OpenLevel(this, LobbyMapPackageName, false);
}

void AShootGameMode::OpenCharacterSelectLevel()
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->ClientTravel(CharacterSelectMapPackageName.ToString(), TRAVEL_Absolute);
		return;
	}

	UGameplayStatics::OpenLevel(this, CharacterSelectMapPackageName, false);
}

void AShootGameMode::OpenGameplayLevel()
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->ClientTravel(GameplayMapPackageName.ToString(), TRAVEL_Absolute);
		return;
	}

	UGameplayStatics::OpenLevel(this, GameplayMapPackageName, false);
}

void AShootGameMode::QuitGameNow()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, true);
}

void AShootGameMode::LoadLeaderboard()
{
	// SaveGame 슬롯에서 기존 기록을 읽어 온다. 저장 파일이 없으면 빈 리더보드로 시작한다.
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

	// 현재 판의 결과를 FLeaderboardEntry로 묶어 메모리 배열에 추가한 뒤 SaveGame 슬롯에 다시 저장한다.
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
	// 점수가 높은 순으로 정렬하고, 동점이면 더 높은 웨이브까지 간 기록을 우선한다.
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
		// 화면에는 Top 5만 필요하므로 저장 데이터도 5개로 제한한다.
		_leaderboardEntries.SetNum(MaxLeaderboardEntries);
	}
}

void AShootGameMode::ApplyMasterVolume()
{
	if (!_masterSoundMix || !_masterSoundClass)
	{
		return;
	}

	// SoundClass 전체에 볼륨 배율을 적용해 BGM/효과음/음성의 기준 볼륨을 함께 조절한다.
	UGameplayStatics::SetSoundMixClassOverride(this, _masterSoundMix, _masterSoundClass, _masterVolume, 1.0f, 0.05f, true);
	UGameplayStatics::PushSoundMixModifier(this, _masterSoundMix);
}

float AShootGameMode::GetCurrentSpawnInterval() const
{
	return GetCurrentWaveDesign().SpawnInterval;
}

const FWaveDesign& AShootGameMode::GetCurrentWaveDesign() const
{
	// 현재 웨이브가 배열 범위를 넘지 않게 Clamp한다.
	// 보스 이후에도 잘못된 인덱스 접근으로 크래시가 나지 않도록 방어한다.
	const int32 Index = FMath::Clamp(_currentWave - 1, 0, _waveDesigns.Num() - 1);
	return _waveDesigns[Index];
}

FVector AShootGameMode::GetSpawnLocation(float ForwardOffset) const
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector PlayerLocation = Player ? Player->GetActorLocation() : FVector::ZeroVector;

	// 적은 항상 플레이어 앞쪽 X 위치에 생성하고, Y만 랜덤으로 흔들어 다른 라인에서 접근하게 한다.
	return FVector(
		PlayerLocation.X + ForwardOffset,
		FMath::RandRange(-760.0f, 760.0f),
		PlayerLocation.Z);
}
