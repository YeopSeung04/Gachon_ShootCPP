#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShootTypes.h"
#include "ShootGameMode.generated.h"

UCLASS()
class SHOOTCPP_API AShootGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AShootGameMode();

protected:
	virtual void BeginPlay() override;

public:
	void OpenLobby();
	void OpenDashboard();
	void OpenShipSelect();
	void SelectFalcon();
	void SelectTitan();
	void StartSelectedGame();
	void RestartGame();
	void QuitGame();
	void TogglePauseMenu();
	void ClosePauseMenu();
	void IncreaseMasterVolume();
	void DecreaseMasterVolume();
	void HandlePrimaryClick(float ScreenX, float ScreenY);
	void RegisterEnemyKilled(int32 ScoreValue, bool IsBoss);
	void NotifyPlayerUltimateUsed();
	void EndGame(bool DidWin);
	void PlayVoiceSound(class USoundBase* Sound, float ExtraDelay = 0.0f);
	void PlayImportantVoiceSound(class USoundBase* Sound);

	EShootGameState GetGameState() const;
	const FPlayerShipData& GetSelectedShipData() const;
	const TArray<FLeaderboardEntry>& GetLeaderboardEntries() const;
	bool IsGameOver() const;
	bool DidPlayerWin() const;
	bool IsBossActive() const;
	float GetBossHealthRatio() const;
	int32 GetCurrentWave() const;
	float GetWaveProgressRatio() const;
	float GetSurvivedTime() const;
	float GetMasterVolume() const;
	bool IsPauseMenuOpen() const;

private:
	// AShootGameMode는 프로젝트의 중앙 흐름 관리자다.
	// 메뉴 상태, 레벨 이동, 웨이브 타이머, 적 스폰, 점수 저장, 사운드 큐를 여기서 통합한다.
	// "게임 규칙"에 가까운 판단은 대부분 이 클래스에서 처리한다.

	// 리더보드에 저장할 최대 기록 수. 점수순으로 정렬한 뒤 5개만 남긴다.
	static constexpr int32 MaxLeaderboardEntries = 5;

	// 일반 적을 몇 마리 처치해야 궁극기 1회를 충전할지 정하는 값.
	static constexpr int32 KillsRequiredForUltimate = 7;

	// 반복 킬 음성인 Gotcha가 너무 자주 나오지 않도록 막는 최소 간격.
	static constexpr float GotchaCalloutCooldown = 4.0f;

	// 일반 적을 Spawn할 때 사용할 C++ 클래스.
	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	TSubclassOf<class AEnemy> _enemyClass;

	// 보스 웨이브에서 Spawn할 보스 클래스.
	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	TSubclassOf<class ABossEnemy> _bossClass;

	// 웨이브 데이터가 없을 때 사용할 기본 적 스폰 간격.
	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	float _baseEnemySpawnInterval = 1.35f;

	// 일반 웨이브 하나가 지속되는 시간. 시간이 지나면 다음 웨이브로 넘어간다.
	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	float _waveDuration = 24.0f;

	// 마지막 보스 웨이브 번호. DataTable을 읽으면 마지막 웨이브 번호로 갱신된다.
	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	int32 _finalBossWave = 5;

	// 플레이어 기체 능력치 DataTable. Falcon/Titan 데이터가 들어 있다.
	UPROPERTY(EditDefaultsOnly, Category="Shoot|Data")
	class UDataTable* _playerStatTable = nullptr;

	// 적 능력치 DataTable. Basic/Fast/Tank/Boss 데이터가 들어 있다.
	UPROPERTY(EditDefaultsOnly, Category="Shoot|Data")
	class UDataTable* _enemyStatTable = nullptr;

	// 웨이브 난이도 DataTable. 스폰 간격, 적 수, 보스 웨이브 여부가 들어 있다.
	UPROPERTY(EditDefaultsOnly, Category="Shoot|Data")
	class UDataTable* _waveDesignTable = nullptr;

	// 일정 시간마다 SpawnEnemy를 호출하는 타이머.
	FTimerHandle _enemySpawnTimerHandle;

	// 웨이브 제한 시간이 끝나면 AdvanceWave를 호출하는 타이머.
	FTimerHandle _waveTimerHandle;

	// 궁극기 충전을 늦게 지급해야 할 때 사용하는 타이머.
	FTimerHandle _ultimateReadyTimerHandle;

	// 일반 음성 큐에서 다음 소리를 지연 재생할 때 쓰는 타이머.
	FTimerHandle _voiceDelayTimerHandle;

	// 킬 콜아웃 큐에서 다음 소리를 지연 재생할 때 쓰는 타이머.
	FTimerHandle _calloutDelayTimerHandle;

	// 레벨 이동을 예약할 때 쓰는 타이머.
	FTimerHandle _levelTransitionTimerHandle;

	// 현재 살아 있는 보스 참조. 보스전 여부와 보스 체력 UI 계산에 사용한다.
	UPROPERTY()
	class ABossEnemy* _bossEnemy = nullptr;

	// 배경 아레나 관리자 참조. 맵에 배치된 SpaceArena를 찾아 보관한다.
	UPROPERTY()
	class ASpaceArena* _spaceArena = nullptr;

	// 메뉴 안내, 시작/종료/궁극기 같은 일반 음성 재생용 컴포넌트.
	UPROPERTY()
	class UAudioComponent* _voiceAudioComponent = nullptr;

	// First Kill, Double Kill 같은 킬 콜아웃 재생용 컴포넌트.
	UPROPERTY()
	class UAudioComponent* _calloutAudioComponent = nullptr;

	// 현재 게임 상태. UI, 입력 모드, 스폰 가능 여부를 결정하는 핵심 상태값이다.
	EShootGameState _gameState = EShootGameState::Lobby;

	// 선택 가능한 두 기체의 실제 능력치 데이터.
	FPlayerShipData _falconData;
	FPlayerShipData _titanData;

	// 현재 플레이어가 선택한 기체 데이터.
	FPlayerShipData _selectedShipData;

	// 적 타입별 능력치 데이터.
	FEnemyStatData _basicEnemyData;
	FEnemyStatData _fastEnemyData;
	FEnemyStatData _tankEnemyData;
	FEnemyStatData _bossEnemyData;

	// 웨이브별 난이도 설계 목록.
	TArray<FWaveDesign> _waveDesigns;

	// SaveGame에서 읽어 온 Top 5 기록.
	TArray<FLeaderboardEntry> _leaderboardEntries;

	// 일반 음성 재생 순서를 저장하는 큐.
	TArray<class USoundBase*> _voiceSoundQueue;
	TArray<float> _voiceDelayQueue;
	TArray<bool> _voiceImportantQueue;

	// 킬 콜아웃 재생 순서를 저장하는 큐.
	TArray<class USoundBase*> _calloutSoundQueue;
	TArray<float> _calloutDelayQueue;

	UPROPERTY()
	class USoundBase* _launchSound = nullptr;

	UPROPERTY()
	class USoundBase* _bossWarningSound = nullptr;

	UPROPERTY()
	class USoundBase* _winSound = nullptr;

	UPROPERTY()
	class USoundBase* _failSound = nullptr;

	UPROPERTY()
	class USoundBase* _firstKillSound = nullptr;

	UPROPERTY()
	class USoundBase* _doubleKillSound = nullptr;

	UPROPERTY()
	class USoundBase* _tripleKillSound = nullptr;

	UPROPERTY()
	class USoundBase* _quadraKillSound = nullptr;

	UPROPERTY()
	class USoundBase* _pentaKillSound = nullptr;

	UPROPERTY()
	class USoundBase* _rampageSound = nullptr;

	UPROPERTY()
	class USoundBase* _gotchaSound = nullptr;

	UPROPERTY()
	class USoundBase* _bossCutSound = nullptr;

	UPROPERTY()
	class USoundBase* _bossClearSound = nullptr;

	UPROPERTY()
	class USoundBase* _clickSound = nullptr;

	UPROPERTY()
	class USoundBase* _gameStartSound = nullptr;

	UPROPERTY()
	class USoundBase* _ultimateReadySound = nullptr;

	UPROPERTY()
	class USoundBase* _quitSound = nullptr;

	UPROPERTY()
	class USoundBase* _selectCharacterSound = nullptr;

	// 현재 진행 중인 웨이브 번호.
	int32 _currentWave = 1;

	// 현재 웨이브에서 처치한 적 수. 킬 콜아웃 단계 계산에 사용한다.
	int32 _killsThisWave = 0;

	// 마지막 궁극기 충전 이후 처치한 일반 적 수.
	int32 _killsSinceLastUltimateCharge = 0;

	// 현재 웨이브에서 지급한 궁극기 횟수.
	int32 _ultimateChargesAwardedThisWave = 0;

	// 아직 플레이어에게 지급하지 않은 궁극기 충전 예약 수.
	int32 _pendingUltimateChargeCount = 0;

	// 예약된 궁극기 충전이 어느 웨이브용인지 확인하는 값.
	int32 _pendingUltimateReadyWave = 0;

	// 현재 웨이브가 시작된 월드 시간.
	float _waveStartTime = 0.0f;

	// 전투가 시작된 월드 시간. 생존 시간 계산에 사용한다.
	float _playStartTime = 0.0f;

	// 일반 음성이 다음에 시작될 수 있는 시간.
	float _nextVoiceSoundTime = 0.0f;

	// 킬 콜아웃이 다음에 시작될 수 있는 시간.
	float _nextCalloutSoundTime = 0.0f;

	// Gotcha 콜아웃을 다시 재생할 수 있는 시간.
	float _nextGotchaCalloutTime = 0.0f;

	// 게임오버 화면에서 승리/패배 문구를 고르는 값.
	bool _didPlayerWin = false;

	// 전체 사운드 볼륨. Pause 메뉴에서 0.0~1.0 사이로 조절한다.
	float _masterVolume = 1.0f;

	UPROPERTY()
	class USoundMix* _masterSoundMix = nullptr;

	UPROPERTY()
	class USoundClass* _masterSoundClass = nullptr;

	void ConfigureShipData();
	void ConfigureEnemyStatData();
	void ConfigureWaveDesigns();
	void LoadDataTables();
	void LoadPersistedShipSelection();
	void SavePersistedShipSelection();
	void InitializeStateForCurrentMap();
	void StartGameplaySession();
	FName GetCurrentMapName() const;
	bool IsCurrentMap(FName MapName) const;
	void QueueLevelTransition(FName MapName);
	void ApplySelectedShipToPlayer();
	void EnsureSpaceArena();
	const FWaveDesign& GetCurrentWaveDesign() const;
	void UpdateInputMode();
	bool IsInsideRect(float ScreenX, float ScreenY, float RectX, float RectY, float RectWidth, float RectHeight) const;
	void StartWave();
	void AdvanceWave();
	void SpawnEnemy();
	void SpawnBoss();
	void ClearEnemies();
	void PlayKillCallout(bool IsBoss);
	void PlayGotchaCallout();
	void PlayMenuClickSound();
	void PlayCalloutSound(class USoundBase* Sound, float ExtraDelay = 0.0f);
	void PlayNextVoiceSound();
	void PlayNextCalloutSound();
	float GetVoiceQueueDelay(float ExtraDelay) const;
	float GetCalloutQueueDelay(float ExtraDelay) const;
	bool HasBlockingCalloutSound() const;
	void EnableUltimateForCurrentWave();
	void QuitGameNow();
	void LoadLeaderboard();
	void SaveLeaderboardEntry(bool DidWin);
	void SortAndTrimLeaderboard();
	void ApplyMasterVolume();
	float GetCurrentSpawnInterval() const;
	FVector GetSpawnLocation(float ForwardOffset) const;

	UFUNCTION()
	void HandleVoiceFinished();

	UFUNCTION()
	void HandleCalloutFinished();

	UFUNCTION()
	void OpenLobbyLevel();

	UFUNCTION()
	void OpenCharacterSelectLevel();

	UFUNCTION()
	void OpenGameplayLevel();
};
