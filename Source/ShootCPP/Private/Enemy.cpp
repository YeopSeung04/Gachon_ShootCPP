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
		Destroy();
	}
}

void AEnemy::InitializeEnemy(float Speed, float MaxHealth, float ContactDamage, int32 ScoreValue, const FVector& MeshScale, const FLinearColor& Color)
{
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
