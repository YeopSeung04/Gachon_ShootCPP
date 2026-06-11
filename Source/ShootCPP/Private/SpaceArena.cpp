#include "SpaceArena.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ASpaceArena::ASpaceArena()
{
	PrimaryActorTick.bCanEverTick = true;

	_rootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = _rootComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Script/Engine.Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));

	if (MaterialFinder.Succeeded())
	{
		_basicMaterial = MaterialFinder.Object;
	}

	if (CubeMeshFinder.Succeeded())
	{
		CreateFloorAndRails(CubeMeshFinder.Object);
		CreateGates(CubeMeshFinder.Object);
	}

	if (SphereMeshFinder.Succeeded())
	{
		CreateStars(SphereMeshFinder.Object);
	}
}

void ASpaceArena::BeginPlay()
{
	Super::BeginPlay();

	for (UStaticMeshComponent* FloorSegment : _floorSegments)
	{
		ApplyColor(FloorSegment, FLinearColor(0.015f, 0.08f, 0.12f, 1.0f));
	}

	for (UStaticMeshComponent* Rail : _leftRails)
	{
		ApplyColor(Rail, FLinearColor(0.0f, 0.45f, 1.0f, 1.0f));
	}

	for (UStaticMeshComponent* Rail : _rightRails)
	{
		ApplyColor(Rail, FLinearColor(0.0f, 0.45f, 1.0f, 1.0f));
	}

	for (UStaticMeshComponent* GateBar : _gateTopBars)
	{
		ApplyColor(GateBar, FLinearColor(0.95f, 0.82f, 0.16f, 1.0f));
	}

	for (UStaticMeshComponent* GateBar : _gateLeftBars)
	{
		ApplyColor(GateBar, FLinearColor(0.95f, 0.82f, 0.16f, 1.0f));
	}

	for (UStaticMeshComponent* GateBar : _gateRightBars)
	{
		ApplyColor(GateBar, FLinearColor(0.95f, 0.82f, 0.16f, 1.0f));
	}

	for (UStaticMeshComponent* Star : _stars)
	{
		ApplyColor(Star, FLinearColor(0.76f, 0.88f, 1.0f, 1.0f));
	}
}

void ASpaceArena::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	const float PlayerX = PlayerPawn->GetActorLocation().X;
	const float ArenaAnchorX = FMath::GridSnap(PlayerX, FloorSegmentSpacing);
	SetActorLocation(FVector(ArenaAnchorX, 0.0f, 0.0f));

	const float LocalPlayerX = PlayerX - ArenaAnchorX;
	const float FloorTotalLength = FloorSegmentCount * FloorSegmentSpacing;
	const float GateTotalLength = GateCount * GateSpacing;

	for (int32 Index = 0; Index < _floorSegments.Num(); ++Index)
	{
		RecycleLinearComponent(_floorSegments[Index], LocalPlayerX, FloorTotalLength);
		RecycleLinearComponent(_leftRails[Index], LocalPlayerX, FloorTotalLength);
		RecycleLinearComponent(_rightRails[Index], LocalPlayerX, FloorTotalLength);
	}

	for (int32 Index = 0; Index < _gateTopBars.Num(); ++Index)
	{
		RecycleLinearComponent(_gateTopBars[Index], LocalPlayerX, GateTotalLength);
		RecycleLinearComponent(_gateLeftBars[Index], LocalPlayerX, GateTotalLength);
		RecycleLinearComponent(_gateRightBars[Index], LocalPlayerX, GateTotalLength);
	}

	for (UStaticMeshComponent* Star : _stars)
	{
		RecycleLinearComponent(Star, LocalPlayerX, FloorTotalLength);
	}
}

void ASpaceArena::CreateFloorAndRails(UStaticMesh* CubeMesh)
{
	for (int32 Index = 0; Index < FloorSegmentCount; ++Index)
	{
		const float X = (Index - FloorSegmentCount / 2) * FloorSegmentSpacing;

		UStaticMeshComponent* FloorSegment = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("FloorSegment_%02d"), Index));
		FloorSegment->SetupAttachment(_rootComponent);
		FloorSegment->SetStaticMesh(CubeMesh);
		FloorSegment->SetRelativeLocation(FVector(X, 0.0f, -60.0f));
		FloorSegment->SetRelativeScale3D(FVector(8.8f, 18.0f, 0.025f));
		if (_basicMaterial)
		{
			FloorSegment->SetMaterial(0, _basicMaterial);
		}
		_floorSegments.Add(FloorSegment);

		UStaticMeshComponent* LeftRail = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("LeftRail_%02d"), Index));
		LeftRail->SetupAttachment(_rootComponent);
		LeftRail->SetStaticMesh(CubeMesh);
		LeftRail->SetRelativeLocation(FVector(X, -1120.0f, 120.0f));
		LeftRail->SetRelativeScale3D(FVector(8.8f, 0.055f, 0.055f));
		if (_basicMaterial)
		{
			LeftRail->SetMaterial(0, _basicMaterial);
		}
		_leftRails.Add(LeftRail);

		UStaticMeshComponent* RightRail = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("RightRail_%02d"), Index));
		RightRail->SetupAttachment(_rootComponent);
		RightRail->SetStaticMesh(CubeMesh);
		RightRail->SetRelativeLocation(FVector(X, 1120.0f, 120.0f));
		RightRail->SetRelativeScale3D(FVector(8.8f, 0.055f, 0.055f));
		if (_basicMaterial)
		{
			RightRail->SetMaterial(0, _basicMaterial);
		}
		_rightRails.Add(RightRail);
	}
}

void ASpaceArena::CreateGates(UStaticMesh* CubeMesh)
{
	for (int32 Index = 0; Index < GateCount; ++Index)
	{
		const float X = (Index - GateCount / 2) * GateSpacing;

		UStaticMeshComponent* TopBar = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("GateTopBar_%02d"), Index));
		TopBar->SetupAttachment(_rootComponent);
		TopBar->SetStaticMesh(CubeMesh);
		TopBar->SetRelativeLocation(FVector(X, 0.0f, 980.0f));
		TopBar->SetRelativeScale3D(FVector(0.08f, 21.0f, 0.08f));
		if (_basicMaterial)
		{
			TopBar->SetMaterial(0, _basicMaterial);
		}
		_gateTopBars.Add(TopBar);

		UStaticMeshComponent* LeftBar = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("GateLeftBar_%02d"), Index));
		LeftBar->SetupAttachment(_rootComponent);
		LeftBar->SetStaticMesh(CubeMesh);
		LeftBar->SetRelativeLocation(FVector(X, -1060.0f, 520.0f));
		LeftBar->SetRelativeScale3D(FVector(0.08f, 0.08f, 9.2f));
		if (_basicMaterial)
		{
			LeftBar->SetMaterial(0, _basicMaterial);
		}
		_gateLeftBars.Add(LeftBar);

		UStaticMeshComponent* RightBar = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("GateRightBar_%02d"), Index));
		RightBar->SetupAttachment(_rootComponent);
		RightBar->SetStaticMesh(CubeMesh);
		RightBar->SetRelativeLocation(FVector(X, 1060.0f, 520.0f));
		RightBar->SetRelativeScale3D(FVector(0.08f, 0.08f, 9.2f));
		if (_basicMaterial)
		{
			RightBar->SetMaterial(0, _basicMaterial);
		}
		_gateRightBars.Add(RightBar);
	}
}

void ASpaceArena::CreateStars(UStaticMesh* SphereMesh)
{
	for (int32 Index = 0; Index < StarCount; ++Index)
	{
		const float X = ((Index % 14) - 7) * FloorSegmentSpacing;
		const float Y = (Index % 2 == 0 ? -1.0f : 1.0f) * (1500.0f + (Index % 5) * 220.0f);
		const float Z = 620.0f + (Index % 7) * 170.0f;

		UStaticMeshComponent* Star = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Star_%02d"), Index));
		Star->SetupAttachment(_rootComponent);
		Star->SetStaticMesh(SphereMesh);
		Star->SetRelativeLocation(FVector(X, Y, Z));
		Star->SetRelativeScale3D(FVector(0.08f + (Index % 3) * 0.035f));
		if (_basicMaterial)
		{
			Star->SetMaterial(0, _basicMaterial);
		}
		_stars.Add(Star);
	}
}

void ASpaceArena::ApplyColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color)
{
	if (!MeshComponent)
	{
		return;
	}

	UMaterialInstanceDynamic* Material = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
		Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}

void ASpaceArena::RecycleLinearComponent(UStaticMeshComponent* MeshComponent, float PlayerX, float TotalLength)
{
	if (!MeshComponent)
	{
		return;
	}

	FVector Location = MeshComponent->GetRelativeLocation();
	const float RecycleDistance = TotalLength * 0.5f;

	if (Location.X < PlayerX - RecycleDistance)
	{
		do
		{
			Location.X += TotalLength;
		}
		while (Location.X < PlayerX - RecycleDistance);

		MeshComponent->SetRelativeLocation(Location);
		return;
	}

	if (Location.X > PlayerX + RecycleDistance)
	{
		do
		{
			Location.X -= TotalLength;
		}
		while (Location.X > PlayerX + RecycleDistance);

		MeshComponent->SetRelativeLocation(Location);
	}
}
