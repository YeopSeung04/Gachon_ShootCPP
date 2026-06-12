#include "BigMissile.h"

#include "CPlayer.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Enemy.h"
#include "UObject/ConstructorHelpers.h"

ABigMissile::ABigMissile()
{
	PrimaryActorTick.bCanEverTick = true;

	_boxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = _boxComponent;
	_boxComponent->SetBoxExtent(FVector(140.0f, 42.0f, 42.0f));
	_boxComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	_bodyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshComponent"));
	_bodyMeshComponent->SetupAttachment(_boxComponent);
	_bodyMeshComponent->SetRelativeScale3D(FVector(2.8f, 0.5f, 0.5f));

	_coreMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreMeshComponent"));
	_coreMeshComponent->SetupAttachment(_boxComponent);
	_coreMeshComponent->SetRelativeLocation(FVector(-70.0f, 0.0f, 0.0f));
	_coreMeshComponent->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (CubeMeshFinder.Succeeded())
	{
		_bodyMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (SphereMeshFinder.Succeeded())
	{
		_coreMeshComponent->SetStaticMesh(SphereMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Script/Engine.Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
	if (MaterialFinder.Succeeded())
	{
		_bodyMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_coreMeshComponent->SetMaterial(0, MaterialFinder.Object);
	}

	InitialLifeSpan = 3.2f;
}

void ABigMissile::BeginPlay()
{
	Super::BeginPlay();

	_boxComponent->OnComponentBeginOverlap.AddDynamic(this, &ABigMissile::OnHit);
}

void ABigMissile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetActorLocation(GetActorLocation() + GetActorForwardVector() * _speed * DeltaTime, true);
}

void ABigMissile::SetDamage(float NewDamage)
{
	_damage = NewDamage;
}

void ABigMissile::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	AEnemy* Enemy = Cast<AEnemy>(OtherActor);
	if (!Enemy || _damagedEnemies.Contains(Enemy))
	{
		return;
	}

	_damagedEnemies.Add(Enemy);
	const float HealthBeforeDamage = Enemy->GetHealth();
	Enemy->ApplyDamage(_damage);
	if (HealthBeforeDamage > 0.0f && Enemy->GetHealth() <= 0.0f)
	{
		if (ACPlayer* Player = Cast<ACPlayer>(GetOwner()))
		{
			Player->HealByMaxHealthPercent(0.05f);
		}
	}
}
