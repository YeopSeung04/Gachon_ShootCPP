#include "Enemy.h"

#include "CPlayer.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "ShootGameMode.h"
#include "UObject/ConstructorHelpers.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// 적도 플레이어처럼 기본 도형을 조합해 만든다.
	// C++ 생성자에서 컴포넌트를 만들면 Blueprint 없이도 월드에 바로 Spawn할 수 있다.
	_boxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = _boxComponent;
	_boxComponent->SetBoxExtent(FVector(55.0f, 55.0f, 35.0f));
	_boxComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	_bodyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshComponent"));
	_bodyMeshComponent->SetupAttachment(_boxComponent);
	_bodyMeshComponent->SetRelativeScale3D(FVector(1.1f, 1.1f, 0.7f));

	_coreMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreMeshComponent"));
	_coreMeshComponent->SetupAttachment(_boxComponent);
	_coreMeshComponent->SetRelativeLocation(FVector(-16.0f, 0.0f, 0.0f));
	_coreMeshComponent->SetRelativeScale3D(FVector(0.34f, 0.34f, 0.34f));

	_leftBladeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftBladeMeshComponent"));
	_leftBladeMeshComponent->SetupAttachment(_boxComponent);
	_leftBladeMeshComponent->SetRelativeLocation(FVector(-4.0f, -52.0f, 0.0f));
	_leftBladeMeshComponent->SetRelativeScale3D(FVector(0.12f, 0.75f, 0.08f));

	_rightBladeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightBladeMeshComponent"));
	_rightBladeMeshComponent->SetupAttachment(_boxComponent);
	_rightBladeMeshComponent->SetRelativeLocation(FVector(-4.0f, 52.0f, 0.0f));
	_rightBladeMeshComponent->SetRelativeScale3D(FVector(0.12f, 0.75f, 0.08f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cone.Cone'"));
	if (MeshFinder.Succeeded())
	{
		_bodyMeshComponent->SetStaticMesh(MeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (SphereMeshFinder.Succeeded())
	{
		_coreMeshComponent->SetStaticMesh(SphereMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (CubeMeshFinder.Succeeded())
	{
		_leftBladeMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
		_rightBladeMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Script/Engine.Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
	if (MaterialFinder.Succeeded())
	{
		_bodyMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_coreMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_leftBladeMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_rightBladeMeshComponent->SetMaterial(0, MaterialFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> HitSoundFinder(TEXT("/Script/Engine.SoundWave'/Engine/VREditor/Sounds/VR_click2.VR_click2'"));
	if (HitSoundFinder.Succeeded())
	{
		_hitSound = HitSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> DestroySoundFinder(TEXT("/Script/Engine.SoundWave'/Engine/EditorSounds/Notifications/CompileFailed.CompileFailed'"));
	if (DestroySoundFinder.Succeeded())
	{
		_destroySound = DestroySoundFinder.Object;
	}
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Spawn 직후 현재 최대 체력으로 시작하고, 색상 변경용 동적 Material을 만든다.
	_health = _maxHealth;
	_enemyMaterial = _bodyMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	_boxComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnHit);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		SetActorLocation(GetActorLocation() + FVector::BackwardVector * _speed * DeltaTime, true);
		return;
	}

	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Z = PlayerLocation.Z;

	// 플레이어의 X/Y 위치를 향해 추적한다. Z는 플레이어 높이에 맞춰 2.5D 슈팅처럼 움직인다.
	const FVector DirectionToPlayer = FVector(
		PlayerLocation.X - CurrentLocation.X,
		PlayerLocation.Y - CurrentLocation.Y,
		0.0f).GetSafeNormal();

	SetActorLocation(CurrentLocation + DirectionToPlayer * _speed * DeltaTime, true);

	if (!DirectionToPlayer.IsNearlyZero())
	{
		SetActorRotation(DirectionToPlayer.Rotation());
	}

	if (GetActorLocation().X < PlayerLocation.X - 2200.0f)
	{
		// 플레이어 뒤로 멀리 지나간 적은 게임에 영향이 없으므로 삭제해 Actor 수를 줄인다.
		Destroy();
	}
}

void AEnemy::InitializeEnemy(float Speed, float MaxHealth, float ContactDamage, int32 ScoreValue, const FVector& MeshScale, const FLinearColor& Color)
{
	// GameMode가 웨이브 난이도와 적 타입에 맞춰 이 함수로 능력치와 외형을 주입한다.
	_speed = Speed;
	_maxHealth = MaxHealth;
	_health = _maxHealth;
	_contactDamage = ContactDamage;
	_scoreValue = ScoreValue;
	_bodyMeshComponent->SetRelativeScale3D(MeshScale);
	_coreMeshComponent->SetRelativeScale3D(MeshScale * 0.3f);
	_leftBladeMeshComponent->SetRelativeScale3D(FVector(0.12f, MeshScale.Y * 0.8f, 0.08f));
	_rightBladeMeshComponent->SetRelativeScale3D(FVector(0.12f, MeshScale.Y * 0.8f, 0.08f));

	if (_enemyMaterial)
	{
		_enemyMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		_enemyMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}

void AEnemy::ApplyDamage(float Damage)
{
	// 탄환/궁극기에서 호출되는 피해 처리다.
	// 체력이 0 이하가 되면 GameMode에 처치 사실을 알리고 점수/콜아웃 처리를 맡긴다.
	_health -= Damage;
	if (_hitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, _hitSound, GetActorLocation(), 0.35f);
	}

	if (_health <= 0.0f)
	{
		DestroyByPlayer();
	}
}

float AEnemy::GetHealthRatio() const
{
	if (_maxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(_health / _maxHealth, 0.0f, 1.0f);
}

float AEnemy::GetHealth() const
{
	return _health;
}

float AEnemy::GetMaxHealth() const
{
	return _maxHealth;
}

int32 AEnemy::GetScoreValue() const
{
	return _scoreValue;
}

void AEnemy::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACPlayer* Player = Cast<ACPlayer>(OtherActor);
	if (!Player)
	{
		return;
	}

	Player->ApplyEnemyDamage(_contactDamage);
	if (!_isBoss)
	{
		// 일반 적은 플레이어와 부딪히면 피해를 주고 사라진다.
		// 보스는 충돌 후에도 전투를 계속해야 하므로 예외로 둔다.
		Destroy();
	}
}

void AEnemy::DestroyByPlayer()
{
	AShootGameMode* GameMode = Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->RegisterEnemyKilled(_scoreValue, _isBoss);
	}

	if (_destroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, _destroySound, GetActorLocation(), 0.5f);
	}

	Destroy();
}
