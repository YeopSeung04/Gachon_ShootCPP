#include "BigMissile.h"

#include "CPlayer.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Enemy.h"
#include "UObject/ConstructorHelpers.h"

ABigMissile::ABigMissile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 궁극기 미사일은 일반 탄환보다 큰 충돌 박스를 가져 여러 적을 관통하며 맞출 수 있다.
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

	// 장시간 남아 불필요한 충돌을 만들지 않도록 수명을 제한한다.
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

	// 궁극기도 탄환과 같은 방식으로 직선 이동하지만 속도와 판정 범위가 다르다.
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

	// 관통형 무기라 같은 적에게 Overlap이 여러 번 발생할 수 있다.
	// TSet으로 이미 맞춘 적을 기록해 한 적당 한 번만 피해를 준다.
	_damagedEnemies.Add(Enemy);
	const float HealthBeforeDamage = Enemy->GetHealth();
	Enemy->ApplyDamage(_damage);
	if (HealthBeforeDamage > 0.0f && Enemy->GetHealth() <= 0.0f)
	{
		// 궁극기로 적을 처치하면 플레이어 최대 체력의 5%를 회복해 리스크 보상을 만든다.
		if (ACPlayer* Player = Cast<ACPlayer>(GetOwner()))
		{
			Player->HealByMaxHealthPercent(0.05f);
		}
	}
}
