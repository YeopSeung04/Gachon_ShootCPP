#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ShootTypes.generated.h"

UENUM(BlueprintType)
enum class EShootGameState : uint8
{
	Lobby,
	Dashboard,
	ShipSelect,
	Playing,
	PauseMenu,
	GameOver
};

UENUM(BlueprintType)
enum class EPlayerShipType : uint8
{
	Falcon,
	Titan
};

USTRUCT(BlueprintType)
struct FPlayerShipData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPlayerShipType ShipType = EPlayerShipType::Falcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName = TEXT("Falcon");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CruiseSpeed = 620.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StrafeSpeed = 1030.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VerticalSpeed = 860.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BulletDamage = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UltimateDamage = 210.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ShipColor = FLinearColor(0.05f, 0.5f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct FEnemyStatData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName = TEXT("Basic Drone");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ContactDamage = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ScoreValue = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MeshScale = FVector(1.1f, 1.1f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(0.55f, 0.9f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct FLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = TEXT("ACE");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShipName = TEXT("Falcon");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wave = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurvivedTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DidWin = false;
};

USTRUCT(BlueprintType)
struct FWaveDesign : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WaveNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnInterval = 1.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnemiesPerSpawn = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasicEnemySpeed = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasicEnemyHealth = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasicEnemyDamage = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastEnemyChance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TankEnemyChance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsBossWave = false;
};
