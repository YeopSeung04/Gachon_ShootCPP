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
	static constexpr int32 MaxLeaderboardEntries = 5;
	static constexpr int32 KillsRequiredForUltimate = 7;
	static constexpr float GotchaCalloutCooldown = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	TSubclassOf<class AEnemy> _enemyClass;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	TSubclassOf<class ABossEnemy> _bossClass;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	float _baseEnemySpawnInterval = 1.35f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	float _waveDuration = 24.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	int32 _finalBossWave = 5;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Data")
	class UDataTable* _playerStatTable = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Data")
	class UDataTable* _enemyStatTable = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Data")
	class UDataTable* _waveDesignTable = nullptr;

	FTimerHandle _enemySpawnTimerHandle;
	FTimerHandle _waveTimerHandle;
	FTimerHandle _ultimateReadyTimerHandle;
	FTimerHandle _voiceDelayTimerHandle;
	FTimerHandle _calloutDelayTimerHandle;
	FTimerHandle _levelTransitionTimerHandle;

	UPROPERTY()
	class ABossEnemy* _bossEnemy = nullptr;

	UPROPERTY()
	class ASpaceArena* _spaceArena = nullptr;

	UPROPERTY()
	class UAudioComponent* _voiceAudioComponent = nullptr;

	UPROPERTY()
	class UAudioComponent* _calloutAudioComponent = nullptr;

	EShootGameState _gameState = EShootGameState::Lobby;
	FPlayerShipData _falconData;
	FPlayerShipData _titanData;
	FPlayerShipData _selectedShipData;
	FEnemyStatData _basicEnemyData;
	FEnemyStatData _fastEnemyData;
	FEnemyStatData _tankEnemyData;
	FEnemyStatData _bossEnemyData;
	TArray<FWaveDesign> _waveDesigns;
	TArray<FLeaderboardEntry> _leaderboardEntries;
	TArray<class USoundBase*> _voiceSoundQueue;
	TArray<float> _voiceDelayQueue;
	TArray<bool> _voiceImportantQueue;
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

	int32 _currentWave = 1;
	int32 _killsThisWave = 0;
	int32 _killsSinceLastUltimateCharge = 0;
	int32 _ultimateChargesAwardedThisWave = 0;
	int32 _pendingUltimateChargeCount = 0;
	int32 _pendingUltimateReadyWave = 0;
	float _waveStartTime = 0.0f;
	float _playStartTime = 0.0f;
	float _nextVoiceSoundTime = 0.0f;
	float _nextCalloutSoundTime = 0.0f;
	float _nextGotchaCalloutTime = 0.0f;
	bool _didPlayerWin = false;
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
