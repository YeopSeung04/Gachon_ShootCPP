#include "BossEnemy.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ABossEnemy::ABossEnemy()
{
	// 보스는 Enemy를 상속받아 체력/피해/점수/충돌 처리를 그대로 사용하고,
	// 이 클래스에서는 크기, 추가 부품, 공격 패턴만 확장한다.
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

	// 시작 위치를 저장하고, 차징 효과를 바꾸기 위한 동적 Material을 만든다.
	_startLocation = GetActorLocation();
	_reactorCoreMaterial = _reactorCoreMeshComponent ? _reactorCoreMeshComponent->CreateAndSetMaterialInstanceDynamic(0) : nullptr;
	UpdateChargeVisual(0.0f);
}

void ABossEnemy::Tick(float DeltaTime)
{
	// _phase는 보스가 좌우로 살짝 흔들리는 움직임을 만들기 위한 누적 시간이다.
	_phase += DeltaTime;

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	// 매 프레임 현재 상태(추적/충전/돌진/복귀)에 맞춰 위치와 시각 효과를 갱신한다.
	UpdateAttackState(DeltaTime, PlayerLocation);
}

void ABossEnemy::UpdateAttackState(float DeltaTime, const FVector& PlayerLocation)
{
	_stateTime += DeltaTime;

	// 보스 패턴은 간단한 상태 머신이다.
	// Tracking으로 플레이어를 따라가다가 Charging으로 예고하고, Dashing으로 공격한 뒤 Retreating으로 물러난다.
	switch (_attackState)
	{
	case EBossAttackState::Tracking:
	{
		// Tracking: 플레이어 앞쪽 위치를 따라다니며 돌진 준비 시간이 끝나길 기다린다.
		_nextChargeTime -= DeltaTime;

		// 플레이어보다 X축으로 앞쪽에 머물고, Y축은 Sin 값으로 흔들어 단조로운 직선 추적을 피한다.
		const FVector TargetLocation(
			PlayerLocation.X + 650.0f,
			PlayerLocation.Y + FMath::Sin(_phase * 1.35f) * 160.0f,
			PlayerLocation.Z);
		MoveToward(TargetLocation, _speed, DeltaTime);

		// 아직 차징 전이므로 코어 크기/발광은 기본 상태로 유지한다.
		UpdateChargeVisual(0.0f);

		if (_nextChargeTime <= 0.0f)
		{
			// 시간이 다 되면 Charging 상태로 넘어가 돌진 예고를 시작한다.
			BeginCharge(PlayerLocation);
		}
		break;
	}
	case EBossAttackState::Charging:
	{
		// Charging: 1.35초 동안 돌진을 예고하는 상태다.
		constexpr float ChargeDuration = 1.35f;

		// _stateTime이 ChargeDuration에 가까워질수록 ChargeRatio가 1에 가까워진다.
		// 이 값이 코어 크기, 색상, 발광 세기를 결정한다.
		const float ChargeRatio = FMath::Clamp(_stateTime / ChargeDuration, 0.0f, 1.0f);
		const FVector ChargeLocation(
			PlayerLocation.X + 720.0f,
			PlayerLocation.Y + FMath::Sin(_phase * 2.1f) * 90.0f,
			PlayerLocation.Z);

		// 차징 중에는 빠르게 공격하지 않고 느리게 위치를 보정해 플레이어에게 반응 시간을 준다.
		MoveToward(ChargeLocation, _speed * 0.45f, DeltaTime);
		UpdateChargeVisual(ChargeRatio);

		if (_stateTime >= ChargeDuration)
		{
			// 차징 시간이 끝나면 실제 돌진 상태로 전환한다.
			BeginDash(PlayerLocation);
		}
		break;
	}
	case EBossAttackState::Dashing:
	{
		// Dashing: 미리 정한 목표 지점까지 매우 빠르게 이동하는 공격 상태다.
		constexpr float DashDuration = 0.82f;
		MoveToward(_dashTargetLocation, 1650.0f, DeltaTime);

		// 돌진 중에는 차징 효과를 최대로 유지한다.
		UpdateChargeVisual(1.0f);

		if (_stateTime >= DashDuration || FVector::DistSquared(GetActorLocation(), _dashTargetLocation) <= FMath::Square(130.0f))
		{
			// 일정 시간이 지나거나 목표 지점에 충분히 가까워지면 복귀 상태로 넘어간다.
			BeginRetreat(PlayerLocation);
		}
		break;
	}
	case EBossAttackState::Retreating:
	{
		// Retreating: 돌진 후 다시 플레이어 앞쪽으로 물러나는 상태다.
		constexpr float RetreatDuration = 1.15f;
		MoveToward(_retreatTargetLocation, 760.0f, DeltaTime);

		// 복귀할수록 차징 효과를 1에서 0으로 줄인다.
		UpdateChargeVisual(FMath::Clamp(1.0f - _stateTime / RetreatDuration, 0.0f, 1.0f));

		if (_stateTime >= RetreatDuration || FVector::DistSquared(GetActorLocation(), _retreatTargetLocation) <= FMath::Square(160.0f))
		{
			// 복귀가 끝나면 다시 Tracking으로 돌아가고, 다음 차징까지 4.2초를 기다린다.
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
	// 돌진 전 예고 단계. 플레이어 근처 위치를 바라보며 코어 발광을 키운다.
	_attackState = EBossAttackState::Charging;
	// 상태에 들어온 시간을 0으로 초기화해야 ChargeRatio가 0부터 다시 증가한다.
	_stateTime = 0.0f;
	// 차징 중에 기준으로 삼을 임시 목표 위치다. 실제 돌진 목표는 BeginDash에서 다시 잡는다.
	_dashTargetLocation = PlayerLocation + FVector(-420.0f, 0.0f, 0.0f);
}

void ABossEnemy::BeginDash(const FVector& PlayerLocation)
{
	// 실제 돌진 목표는 플레이어 뒤쪽까지 잡아 충돌 압박을 만든다.
	_attackState = EBossAttackState::Dashing;
	// Dashing 지속 시간을 새로 재기 위해 상태 시간을 초기화한다.
	_stateTime = 0.0f;
	// 플레이어 현재 위치보다 뒤쪽까지 목표를 잡아, 보스가 플레이어를 스쳐 지나가듯 돌진한다.
	_dashTargetLocation = PlayerLocation + FVector(-520.0f, 0.0f, 0.0f);
}

void ABossEnemy::BeginRetreat(const FVector& PlayerLocation)
{
	// 돌진 후 다시 플레이어 앞쪽으로 돌아가 다음 패턴을 준비한다.
	_attackState = EBossAttackState::Retreating;
	// Retreating 지속 시간을 새로 재기 위해 상태 시간을 초기화한다.
	_stateTime = 0.0f;
	// 플레이어 앞쪽 X 위치로 빠지되, Y축을 조금 흔들어 다음 추적 위치가 매번 같지 않게 한다.
	_retreatTargetLocation = PlayerLocation + FVector(780.0f, FMath::Sin(_phase) * 180.0f, 0.0f);
}

void ABossEnemy::MoveToward(const FVector& TargetLocation, float MoveSpeed, float DeltaTime)
{
	FVector CurrentLocation = GetActorLocation();
	FVector AdjustedTarget = TargetLocation;

	// 보스는 플레이어와 같은 높이에서 싸우도록 Z를 목표 위치에 맞춘다.
	CurrentLocation.Z = TargetLocation.Z;
	AdjustedTarget.Z = TargetLocation.Z;

	// 현재 위치에서 목표 위치로 향하는 방향 벡터를 구한다.
	const FVector Direction = (AdjustedTarget - CurrentLocation).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		return;
	}

	// 이동 공식: 현재 위치 + 방향 * 이동속도 * DeltaTime
	// 이동 후에는 보스의 앞 방향도 이동 방향을 바라보게 회전시킨다.
	SetActorLocation(CurrentLocation + Direction * MoveSpeed * DeltaTime, true);
	SetActorRotation(Direction.Rotation());
}

void ABossEnemy::UpdateChargeVisual(float ChargeRatio)
{
	// ChargeRatio가 0에서 1로 갈수록 코어가 커지고 색/발광이 바뀌어 돌진 타이밍을 예고한다.
	// ChargeRatio 0: 평상시, ChargeRatio 1: 돌진 직전 또는 돌진 중 최대 경고 상태.
	const float Scale = FMath::Lerp(0.62f, 1.35f, ChargeRatio);
	if (_reactorCoreMeshComponent)
	{
		// Lerp 결과로 코어 Mesh 크기를 키워 차징이 진행 중임을 눈으로 보여준다.
		_reactorCoreMeshComponent->SetRelativeScale3D(FVector(Scale));
	}

	if (_reactorCoreMaterial)
	{
		// HSV 보간으로 빨간색에서 청록색 쪽으로 색을 바꾼다.
		const FLinearColor CoreColor = FLinearColor::LerpUsingHSV(
			FLinearColor(0.95f, 0.16f, 0.1f, 1.0f),
			FLinearColor(0.0f, 0.85f, 1.0f, 1.0f),
			ChargeRatio);
		_reactorCoreMaterial->SetVectorParameterValue(TEXT("Color"), CoreColor);
		_reactorCoreMaterial->SetVectorParameterValue(TEXT("BaseColor"), CoreColor);
		// EmissiveColor를 키워 차징이 강해질수록 더 밝게 보이게 한다.
		_reactorCoreMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), CoreColor * FMath::Lerp(0.0f, 9.0f, ChargeRatio));
	}
}
