#pragma once

#include "CoreMinimal.h"
#include "ShootTypes.generated.h"

UENUM(BlueprintType)
enum class EShootGameState : uint8
{
	Lobby,
	Dashboard,
	ShipSelect,
	Playing,
	GameOver
};

UENUM(BlueprintType)
enum class EPlayerShipType : uint8
{
	Falcon,
	Titan
};

USTRUCT(BlueprintType)
struct FPlayerShipData
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
	float FireInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ShipColor = FLinearColor(0.05f, 0.5f, 1.0f, 1.0f);
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
struct FWaveDesign
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
