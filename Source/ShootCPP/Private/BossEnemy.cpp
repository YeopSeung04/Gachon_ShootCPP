#include "BossEnemy.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ABossEnemy::ABossEnemy()
{
	_speed = 260.0f;
	_maxHealth = 760.0f;
	_contactDamage = 72.0f;
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
	_reactorCoreMaterial = _reactorCoreMeshComponent ? _reactorCoreMeshComponent->CreateAndSetMaterialInstanceDynamic(0) : nullptr;
	UpdateChargeVisual(0.0f);
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
	UpdateAttackState(DeltaTime, PlayerLocation);
}

void ABossEnemy::UpdateAttackState(float DeltaTime, const FVector& PlayerLocation)
{
	_stateTime += DeltaTime;

	switch (_attackState)
	{
	case EBossAttackState::Tracking:
	{
		_nextChargeTime -= DeltaTime;
		const FVector TargetLocation(
			PlayerLocation.X + 650.0f,
			PlayerLocation.Y + FMath::Sin(_phase * 1.35f) * 160.0f,
			PlayerLocation.Z);
		MoveToward(TargetLocation, _speed, DeltaTime);
		UpdateChargeVisual(0.0f);

		if (_nextChargeTime <= 0.0f)
		{
			BeginCharge(PlayerLocation);
		}
		break;
	}
	case EBossAttackState::Charging:
	{
		constexpr float ChargeDuration = 1.35f;
		const float ChargeRatio = FMath::Clamp(_stateTime / ChargeDuration, 0.0f, 1.0f);
		const FVector ChargeLocation(
			PlayerLocation.X + 720.0f,
			PlayerLocation.Y + FMath::Sin(_phase * 2.1f) * 90.0f,
			PlayerLocation.Z);
		MoveToward(ChargeLocation, _speed * 0.45f, DeltaTime);
		UpdateChargeVisual(ChargeRatio);

		if (_stateTime >= ChargeDuration)
		{
			BeginDash(PlayerLocation);
		}
		break;
	}
	case EBossAttackState::Dashing:
	{
		constexpr float DashDuration = 0.82f;
		MoveToward(_dashTargetLocation, 1650.0f, DeltaTime);
		UpdateChargeVisual(1.0f);

		if (_stateTime >= DashDuration || FVector::DistSquared(GetActorLocation(), _dashTargetLocation) <= FMath::Square(130.0f))
		{
			BeginRetreat(PlayerLocation);
		}
		break;
	}
	case EBossAttackState::Retreating:
	{
		constexpr float RetreatDuration = 1.15f;
		MoveToward(_retreatTargetLocation, 760.0f, DeltaTime);
		UpdateChargeVisual(FMath::Clamp(1.0f - _stateTime / RetreatDuration, 0.0f, 1.0f));

		if (_stateTime >= RetreatDuration || FVector::DistSquared(GetActorLocation(), _retreatTargetLocation) <= FMath::Square(160.0f))
		{
			_attackState = EBossAttackState::Tracking;
			_stateTime = 0.0f;
			_nextChargeTime = 4.2f;
			UpdateChargeVisual(0.0f);
		}
		break;
	}
	default:
		break;
	}
}

void ABossEnemy::BeginCharge(const FVector& PlayerLocation)
{
	_attackState = EBossAttackState::Charging;
	_stateTime = 0.0f;
	_dashTargetLocation = PlayerLocation + FVector(-420.0f, 0.0f, 0.0f);
}

void ABossEnemy::BeginDash(const FVector& PlayerLocation)
{
	_attackState = EBossAttackState::Dashing;
	_stateTime = 0.0f;
	_dashTargetLocation = PlayerLocation + FVector(-520.0f, 0.0f, 0.0f);
}

void ABossEnemy::BeginRetreat(const FVector& PlayerLocation)
{
	_attackState = EBossAttackState::Retreating;
	_stateTime = 0.0f;
	_retreatTargetLocation = PlayerLocation + FVector(780.0f, FMath::Sin(_phase) * 180.0f, 0.0f);
}

void ABossEnemy::MoveToward(const FVector& TargetLocation, float MoveSpeed, float DeltaTime)
{
	FVector CurrentLocation = GetActorLocation();
	FVector AdjustedTarget = TargetLocation;
	CurrentLocation.Z = TargetLocation.Z;
	AdjustedTarget.Z = TargetLocation.Z;

	const FVector Direction = (AdjustedTarget - CurrentLocation).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		return;
	}

	SetActorLocation(CurrentLocation + Direction * MoveSpeed * DeltaTime, true);
	SetActorRotation(Direction.Rotation());
}

void ABossEnemy::UpdateChargeVisual(float ChargeRatio)
{
	const float Scale = FMath::Lerp(0.62f, 1.35f, ChargeRatio);
	if (_reactorCoreMeshComponent)
	{
		_reactorCoreMeshComponent->SetRelativeScale3D(FVector(Scale));
	}

	if (_reactorCoreMaterial)
	{
		const FLinearColor CoreColor = FLinearColor::LerpUsingHSV(
			FLinearColor(0.95f, 0.16f, 0.1f, 1.0f),
			FLinearColor(0.0f, 0.85f, 1.0f, 1.0f),
			ChargeRatio);
		_reactorCoreMaterial->SetVectorParameterValue(TEXT("Color"), CoreColor);
		_reactorCoreMaterial->SetVectorParameterValue(TEXT("BaseColor"), CoreColor);
		_reactorCoreMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), CoreColor * FMath::Lerp(0.0f, 9.0f, ChargeRatio));
	}
}
