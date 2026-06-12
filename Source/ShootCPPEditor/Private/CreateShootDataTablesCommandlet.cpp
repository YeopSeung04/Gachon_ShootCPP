#include "CreateShootDataTablesCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Engine/DataTable.h"
#include "ShootTypes.h"

namespace
{
	static UDataTable* CreateOrResetDataTable(const TCHAR* PackagePath, UScriptStruct* RowStruct, TArray<UPackage*>& PackagesToSave)
	{
		UPackage* Package = CreatePackage(PackagePath);
		const FString AssetName = FPackageName::GetLongPackageAssetName(PackagePath);

		UDataTable* DataTable = FindObject<UDataTable>(Package, *AssetName);
		if (!DataTable)
		{
			DataTable = NewObject<UDataTable>(Package, *AssetName, RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(DataTable);
		}

		DataTable->Modify();
		DataTable->RowStruct = RowStruct;
		DataTable->EmptyTable();
		DataTable->MarkPackageDirty();
		PackagesToSave.AddUnique(Package);
		return DataTable;
	}

	static void PopulatePlayerStats(TArray<UPackage*>& PackagesToSave)
	{
		UDataTable* Table = CreateOrResetDataTable(TEXT("/Game/Data/DT_PlayerStats"), FPlayerShipData::StaticStruct(), PackagesToSave);
		if (!Table)
		{
			return;
		}

		FPlayerShipData Falcon;
		Falcon.ShipType = EPlayerShipType::Falcon;
		Falcon.DisplayName = TEXT("Falcon");
		Falcon.CruiseSpeed = 640.0f;
		Falcon.StrafeSpeed = 1080.0f;
		Falcon.VerticalSpeed = 900.0f;
		Falcon.MaxHealth = 90.0f;
		Falcon.BulletDamage = 16.0f;
		Falcon.UltimateDamage = 210.0f;
		Falcon.FireInterval = 0.08f;
		Falcon.ShipColor = FLinearColor(0.0f, 0.55f, 1.0f, 1.0f);
		Table->AddRow(TEXT("Falcon"), Falcon);

		FPlayerShipData Titan;
		Titan.ShipType = EPlayerShipType::Titan;
		Titan.DisplayName = TEXT("Titan");
		Titan.CruiseSpeed = 500.0f;
		Titan.StrafeSpeed = 760.0f;
		Titan.VerticalSpeed = 650.0f;
		Titan.MaxHealth = 170.0f;
		Titan.BulletDamage = 34.0f;
		Titan.UltimateDamage = 260.0f;
		Titan.FireInterval = 0.18f;
		Titan.ShipColor = FLinearColor(1.0f, 0.22f, 0.04f, 1.0f);
		Table->AddRow(TEXT("Titan"), Titan);
	}

	static void PopulateEnemyStats(TArray<UPackage*>& PackagesToSave)
	{
		UDataTable* Table = CreateOrResetDataTable(TEXT("/Game/Data/DT_EnemyStats"), FEnemyStatData::StaticStruct(), PackagesToSave);
		if (!Table)
		{
			return;
		}

		FEnemyStatData Basic;
		Basic.DisplayName = TEXT("Basic Drone");
		Basic.MoveSpeed = 560.0f;
		Basic.MaxHealth = 35.0f;
		Basic.ContactDamage = 14.0f;
		Basic.ScoreValue = 100;
		Basic.MeshScale = FVector(1.1f, 1.1f, 0.7f);
		Basic.Color = FLinearColor(0.55f, 0.9f, 1.0f, 1.0f);
		Table->AddRow(TEXT("Basic"), Basic);

		FEnemyStatData Fast;
		Fast.DisplayName = TEXT("Red Fang");
		Fast.MoveSpeed = 795.0f;
		Fast.MaxHealth = 28.0f;
		Fast.ContactDamage = 16.0f;
		Fast.ScoreValue = 170;
		Fast.MeshScale = FVector(0.86f, 0.86f, 0.5f);
		Fast.Color = FLinearColor(1.0f, 0.08f, 0.1f, 1.0f);
		Table->AddRow(TEXT("Fast"), Fast);

		FEnemyStatData Tank;
		Tank.DisplayName = TEXT("Iron Guard");
		Tank.MoveSpeed = 405.0f;
		Tank.MaxHealth = 75.0f;
		Tank.ContactDamage = 20.0f;
		Tank.ScoreValue = 260;
		Tank.MeshScale = FVector(1.65f, 1.65f, 1.05f);
		Tank.Color = FLinearColor(0.95f, 0.72f, 0.12f, 1.0f);
		Table->AddRow(TEXT("Tank"), Tank);

		FEnemyStatData Boss;
		Boss.DisplayName = TEXT("Command Carrier");
		Boss.MoveSpeed = 260.0f;
		Boss.MaxHealth = 760.0f;
		Boss.ContactDamage = 72.0f;
		Boss.ScoreValue = 2500;
		Boss.MeshScale = FVector(3.2f, 2.6f, 1.7f);
		Boss.Color = FLinearColor(0.95f, 0.16f, 0.1f, 1.0f);
		Table->AddRow(TEXT("Boss"), Boss);
	}

	static void PopulateWaveDesigns(TArray<UPackage*>& PackagesToSave)
	{
		UDataTable* Table = CreateOrResetDataTable(TEXT("/Game/Data/DT_WaveDesigns"), FWaveDesign::StaticStruct(), PackagesToSave);
		if (!Table)
		{
			return;
		}

		FWaveDesign Wave1;
		Wave1.WaveNumber = 1;
		Wave1.SpawnInterval = 1.45f;
		Wave1.EnemiesPerSpawn = 1;
		Wave1.BasicEnemySpeed = 560.0f;
		Wave1.BasicEnemyHealth = 35.0f;
		Wave1.BasicEnemyDamage = 14.0f;
		Table->AddRow(TEXT("Wave_01"), Wave1);

		FWaveDesign Wave2;
		Wave2.WaveNumber = 2;
		Wave2.SpawnInterval = 1.2f;
		Wave2.EnemiesPerSpawn = 1;
		Wave2.BasicEnemySpeed = 640.0f;
		Wave2.BasicEnemyHealth = 45.0f;
		Wave2.BasicEnemyDamage = 18.0f;
		Table->AddRow(TEXT("Wave_02"), Wave2);

		FWaveDesign Wave3;
		Wave3.WaveNumber = 3;
		Wave3.SpawnInterval = 1.0f;
		Wave3.EnemiesPerSpawn = 2;
		Wave3.BasicEnemySpeed = 720.0f;
		Wave3.BasicEnemyHealth = 52.0f;
		Wave3.BasicEnemyDamage = 20.0f;
		Wave3.FastEnemyChance = 0.42f;
		Table->AddRow(TEXT("Wave_03"), Wave3);

		FWaveDesign Wave4;
		Wave4.WaveNumber = 4;
		Wave4.SpawnInterval = 0.82f;
		Wave4.EnemiesPerSpawn = 2;
		Wave4.BasicEnemySpeed = 790.0f;
		Wave4.BasicEnemyHealth = 62.0f;
		Wave4.BasicEnemyDamage = 24.0f;
		Wave4.FastEnemyChance = 0.35f;
		Wave4.TankEnemyChance = 0.24f;
		Table->AddRow(TEXT("Wave_04"), Wave4);

		FWaveDesign Wave5;
		Wave5.WaveNumber = 5;
		Wave5.SpawnInterval = 999.0f;
		Wave5.IsBossWave = true;
		Table->AddRow(TEXT("Wave_05_Boss"), Wave5);
	}
}

UCreateShootDataTablesCommandlet::UCreateShootDataTablesCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UCreateShootDataTablesCommandlet::Main(const FString& Params)
{
	TArray<UPackage*> PackagesToSave;

	PopulatePlayerStats(PackagesToSave);
	PopulateEnemyStats(PackagesToSave);
	PopulateWaveDesigns(PackagesToSave);

	UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
	UE_LOG(LogTemp, Display, TEXT("Created shoot gameplay DataTables."));
	return 0;
}
