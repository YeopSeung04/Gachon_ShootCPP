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
	void HandlePrimaryClick(float ScreenX, float ScreenY);
	void RegisterEnemyKilled(int32 ScoreValue, bool IsBoss);
	void EndGame(bool DidWin);

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

private:
	static constexpr int32 MaxLeaderboardEntries = 5;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	TSubclassOf<class AEnemy> _enemyClass;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	TSubclassOf<class ABossEnemy> _bossClass;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	float _baseEnemySpawnInterval = 1.35f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	float _waveDuration = 24.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Spawn")
	int32 _finalBossWave = 6;

	FTimerHandle _enemySpawnTimerHandle;
	FTimerHandle _waveTimerHandle;

	UPROPERTY()
	class ABossEnemy* _bossEnemy = nullptr;

	UPROPERTY()
	class ASpaceArena* _spaceArena = nullptr;

	EShootGameState _gameState = EShootGameState::Lobby;
	FPlayerShipData _falconData;
	FPlayerShipData _titanData;
	FPlayerShipData _selectedShipData;
	TArray<FWaveDesign> _waveDesigns;
	TArray<FLeaderboardEntry> _leaderboardEntries;

	UPROPERTY()
	class USoundBase* _launchSound = nullptr;

	UPROPERTY()
	class USoundBase* _bossWarningSound = nullptr;

	UPROPERTY()
	class USoundBase* _winSound = nullptr;

	UPROPERTY()
	class USoundBase* _failSound = nullptr;

	int32 _currentWave = 1;
	float _waveStartTime = 0.0f;
	float _playStartTime = 0.0f;
	bool _didPlayerWin = false;

	void ConfigureShipData();
	void ConfigureWaveDesigns();
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
	void LoadLeaderboard();
	void SaveLeaderboardEntry(bool DidWin);
	void SortAndTrimLeaderboard();
	float GetCurrentSpawnInterval() const;
	FVector GetSpawnLocation(float ForwardOffset) const;
};
