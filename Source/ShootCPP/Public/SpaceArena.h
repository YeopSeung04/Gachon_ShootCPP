#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceArena.generated.h"

UCLASS()
class SHOOTCPP_API ASpaceArena : public AActor
{
	GENERATED_BODY()

public:
	ASpaceArena();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	static constexpr int32 FloorSegmentCount = 14;
	static constexpr int32 GateCount = 10;
	static constexpr int32 StarCount = 28;
	static constexpr float FloorSegmentSpacing = 950.0f;
	static constexpr float GateSpacing = 1450.0f;
	static constexpr float RecycleBehindDistance = 1800.0f;

	UPROPERTY(VisibleAnywhere, Category="Arena")
	class USceneComponent* _rootComponent;

	UPROPERTY()
	TArray<class UStaticMeshComponent*> _floorSegments;

	UPROPERTY()
	TArray<class UStaticMeshComponent*> _leftRails;

	UPROPERTY()
	TArray<class UStaticMeshComponent*> _rightRails;

	UPROPERTY()
	TArray<class UStaticMeshComponent*> _gateTopBars;

	UPROPERTY()
	TArray<class UStaticMeshComponent*> _gateLeftBars;

	UPROPERTY()
	TArray<class UStaticMeshComponent*> _gateRightBars;

	UPROPERTY()
	TArray<class UStaticMeshComponent*> _stars;

	UPROPERTY()
	class UMaterialInterface* _basicMaterial = nullptr;

	void CreateFloorAndRails(UStaticMesh* CubeMesh);
	void CreateGates(UStaticMesh* CubeMesh);
	void CreateStars(UStaticMesh* SphereMesh);
	void ApplyColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color);
	void RecycleLinearComponent(UStaticMeshComponent* MeshComponent, float PlayerX, float TotalLength);
};
