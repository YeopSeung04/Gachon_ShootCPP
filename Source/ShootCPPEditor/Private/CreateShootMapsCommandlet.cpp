#include "CreateShootMapsCommandlet.h"

#include "Editor.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "SpaceArena.h"

namespace
{
	const TCHAR* SourceMapPackageName = TEXT("/Game/Maps/ShootingMap");
	const TCHAR* LobbyMapPackageName = TEXT("/Game/Maps/LobbyMap");
	const TCHAR* CharacterSelectMapPackageName = TEXT("/Game/Maps/CharacterSelectMap");
	const TCHAR* GameplayMapPackageName = TEXT("/Game/Maps/GameplayMap");

	const FName GeneratedArenaTag(TEXT("GeneratedArena"));
	const FName FloorSegmentTag(TEXT("ArenaFloorSegment"));
	const FName LeftRailTag(TEXT("ArenaLeftRail"));
	const FName RightRailTag(TEXT("ArenaRightRail"));
	const FName GateTopTag(TEXT("ArenaGateTop"));
	const FName GateLeftTag(TEXT("ArenaGateLeft"));
	const FName GateRightTag(TEXT("ArenaGateRight"));
	const FName StarTag(TEXT("ArenaStar"));

	constexpr int32 FloorSegmentCount = 14;
	constexpr int32 GateCount = 10;
	constexpr int32 StarCount = 28;
	constexpr float FloorSegmentSpacing = 950.0f;
	constexpr float GateSpacing = 1450.0f;

	static bool ResolveMapFilename(const FString& PackageName, FString& OutFilename)
	{
		return FPackageName::TryConvertLongPackageNameToFilename(PackageName, OutFilename, FPackageName::GetMapPackageExtension());
	}

	static bool CopySourceMapIfNeeded(const FString& DestinationPackageName)
	{
		FString SourceFilename;
		FString DestinationFilename;
		if (!ResolveMapFilename(SourceMapPackageName, SourceFilename) || !ResolveMapFilename(DestinationPackageName, DestinationFilename))
		{
			return false;
		}

		if (FPaths::FileExists(DestinationFilename))
		{
			return true;
		}

		const uint32 CopyResult = IFileManager::Get().Copy(*DestinationFilename, *SourceFilename);
		if (CopyResult != COPY_OK)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to copy %s to %s"), *SourceFilename, *DestinationFilename);
			return false;
		}

		UE_LOG(LogTemp, Display, TEXT("Created map asset: %s"), *DestinationPackageName);
		return true;
	}

	static void ClearGeneratedActors(UWorld* World)
	{
		TArray<AActor*> ActorsToDestroy;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->Tags.Contains(GeneratedArenaTag))
			{
				ActorsToDestroy.Add(Actor);
			}
		}

		for (AActor* Actor : ActorsToDestroy)
		{
			World->EditorDestroyActor(Actor, true);
		}
	}

	static AStaticMeshActor* AddMeshActor(UWorld* World, UStaticMesh* Mesh, UMaterialInterface* Material, const FString& Label, FName ArenaTag, const FVector& Location, const FVector& Scale)
	{
		AActor* NewActor = GEditor->AddActor(World->GetCurrentLevel(), AStaticMeshActor::StaticClass(), FTransform(Location));
		AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(NewActor);
		if (!MeshActor)
		{
			return nullptr;
		}

		MeshActor->SetActorLabel(Label);
		MeshActor->Tags.AddUnique(GeneratedArenaTag);
		MeshActor->Tags.AddUnique(ArenaTag);
		MeshActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
		MeshActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
		MeshActor->GetStaticMeshComponent()->SetWorldScale3D(Scale);
		if (Material)
		{
			MeshActor->GetStaticMeshComponent()->SetMaterial(0, Material);
		}
		return MeshActor;
	}

	static void AddArenaGeometry(UWorld* World, const FString& LabelPrefix)
	{
		UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
		UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
		UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Script/Engine.Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
		if (!CubeMesh || !SphereMesh)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not load engine basic meshes."));
			return;
		}

		AActor* ArenaManager = GEditor->AddActor(World->GetCurrentLevel(), ASpaceArena::StaticClass(), FTransform::Identity);
		if (ArenaManager)
		{
			ArenaManager->SetActorLabel(FString::Printf(TEXT("%s_ArenaManager"), *LabelPrefix));
			ArenaManager->Tags.AddUnique(GeneratedArenaTag);
		}

		for (int32 Index = 0; Index < FloorSegmentCount; ++Index)
		{
			const float X = (Index - FloorSegmentCount / 2) * FloorSegmentSpacing;
			AddMeshActor(World, CubeMesh, Material, FString::Printf(TEXT("%s_Floor_%02d"), *LabelPrefix, Index), FloorSegmentTag, FVector(X, 0.0f, -60.0f), FVector(8.8f, 18.0f, 0.025f));
			AddMeshActor(World, CubeMesh, Material, FString::Printf(TEXT("%s_LeftRail_%02d"), *LabelPrefix, Index), LeftRailTag, FVector(X, -1120.0f, 120.0f), FVector(8.8f, 0.055f, 0.055f));
			AddMeshActor(World, CubeMesh, Material, FString::Printf(TEXT("%s_RightRail_%02d"), *LabelPrefix, Index), RightRailTag, FVector(X, 1120.0f, 120.0f), FVector(8.8f, 0.055f, 0.055f));
		}

		for (int32 Index = 0; Index < GateCount; ++Index)
		{
			const float X = (Index - GateCount / 2) * GateSpacing;
			AddMeshActor(World, CubeMesh, Material, FString::Printf(TEXT("%s_GateTop_%02d"), *LabelPrefix, Index), GateTopTag, FVector(X, 0.0f, 980.0f), FVector(0.08f, 21.0f, 0.08f));
			AddMeshActor(World, CubeMesh, Material, FString::Printf(TEXT("%s_GateLeft_%02d"), *LabelPrefix, Index), GateLeftTag, FVector(X, -1060.0f, 520.0f), FVector(0.08f, 0.08f, 9.2f));
			AddMeshActor(World, CubeMesh, Material, FString::Printf(TEXT("%s_GateRight_%02d"), *LabelPrefix, Index), GateRightTag, FVector(X, 1060.0f, 520.0f), FVector(0.08f, 0.08f, 9.2f));
		}

		for (int32 Index = 0; Index < StarCount; ++Index)
		{
			const float X = ((Index % 14) - 7) * FloorSegmentSpacing;
			const float Y = (Index % 2 == 0 ? -1.0f : 1.0f) * (1500.0f + (Index % 5) * 220.0f);
			const float Z = 620.0f + (Index % 7) * 170.0f;
			AddMeshActor(World, SphereMesh, Material, FString::Printf(TEXT("%s_Star_%02d"), *LabelPrefix, Index), StarTag, FVector(X, Y, Z), FVector(0.08f + (Index % 3) * 0.035f));
		}
	}

	static bool PopulateMap(const FString& MapPackageName, const FString& LabelPrefix)
	{
		if (!CopySourceMapIfNeeded(MapPackageName))
		{
			return false;
		}

		FString MapFilename;
		if (!ResolveMapFilename(MapPackageName, MapFilename))
		{
			return false;
		}

		if (!FEditorFileUtils::LoadMap(MapFilename, false, true))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load map: %s"), *MapPackageName);
			return false;
		}

		UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (!World)
		{
			return false;
		}

		ClearGeneratedActors(World);
		AddArenaGeometry(World, LabelPrefix);

		return FEditorFileUtils::SaveLevel(World->PersistentLevel);
	}
}

UCreateShootMapsCommandlet::UCreateShootMapsCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UCreateShootMapsCommandlet::Main(const FString& Params)
{
	const bool bLobbyOk = PopulateMap(LobbyMapPackageName, TEXT("Lobby"));
	const bool bCharacterSelectOk = PopulateMap(CharacterSelectMapPackageName, TEXT("CharacterSelect"));
	const bool bGameplayOk = PopulateMap(GameplayMapPackageName, TEXT("Gameplay"));

	return bLobbyOk && bCharacterSelectOk && bGameplayOk ? 0 : 1;
}
