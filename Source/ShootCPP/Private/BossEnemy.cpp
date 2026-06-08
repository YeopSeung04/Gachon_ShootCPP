#include "BossEnemy.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ABossEnemy::ABossEnemy()
{
	_speed = 105.0f;
	_maxHealth = 760.0f;
	_contactDamage = 55.0f;
	_scoreValue = 2500;
	_isBoss = true;

	if (_boxComponent)
	{
		_boxComponent->SetBoxExtent(FVector(160.0f, 130.0f, 85.0f));
	}

	if (_bodyMeshComponent)
	{
		_bodyMeshComponent->SetRelativeScale3D(FVector(3.2f, 2.6f, 1.7f));
	}

	_carrierDeckMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarrierDeckMeshComponent"));
	_carrierDeckMeshComponent->SetupAttachment(_boxComponent);
	_carrierDeckMeshComponent->SetRelativeLocation(FVector(-55.0f, 0.0f, -10.0f));
	_carrierDeckMeshComponent->SetRelativeScale3D(FVector(2.8f, 5.2f, 0.22f));

	_leftHangarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftHangarMeshComponent"));
	_leftHangarMeshComponent->SetupAttachment(_boxComponent);
	_leftHangarMeshComponent->SetRelativeLocation(FVector(-95.0f, -132.0f, 18.0f));
	_leftHangarMeshComponent->SetRelativeScale3D(FVector(1.5f, 0.48f, 0.48f));

	_rightHangarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightHangarMeshComponent"));
	_rightHangarMeshComponent->SetupAttachment(_boxComponent);
	_rightHangarMeshComponent->SetRelativeLocation(FVector(-95.0f, 132.0f, 18.0f));
	_rightHangarMeshComponent->SetRelativeScale3D(FVector(1.5f, 0.48f, 0.48f));

	_reactorCoreMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ReactorCoreMeshComponent"));
	_reactorCoreMeshComponent->SetupAttachment(_boxComponent);
	_reactorCoreMeshComponent->SetRelativeLocation(FVector(20.0f, 0.0f, 64.0f));
	_reactorCoreMeshComponent->SetRelativeScale3D(FVector(0.62f, 0.62f, 0.62f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (CubeMeshFinder.Succeeded())
	{
		_carrierDeckMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
		_leftHangarMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
		_rightHangarMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (SphereMeshFinder.Succeeded())
	{
		_reactorCoreMeshComponent->SetStaticMesh(SphereMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Script/Engine.Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
	if (MaterialFinder.Succeeded())
	{
		_carrierDeckMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_leftHangarMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_rightHangarMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_reactorCoreMeshComponent->SetMaterial(0, MaterialFinder.Object);
	}
}

void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();

	_startLocation = GetActorLocation();
}

void ABossEnemy::Tick(float DeltaTime)
{
	_phase += DeltaTime;

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Z = PlayerLocation.Z;

	const FVector TargetLocation(
		PlayerLocation.X,
		PlayerLocation.Y + FMath::Sin(_phase * 1.35f) * 120.0f,
		PlayerLocation.Z);
	const FVector DirectionToPlayer = (TargetLocation - CurrentLocation).GetSafeNormal();

	SetActorLocation(CurrentLocation + DirectionToPlayer * _speed * DeltaTime, true);

	if (!DirectionToPlayer.IsNearlyZero())
	{
		SetActorRotation(DirectionToPlayer.Rotation());
	}
}
