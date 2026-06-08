#include "Bullet.h"

#include "Components/BoxComponent.h"
#include "Enemy.h"

ABullet::ABullet()
{
	PrimaryActorTick.bCanEverTick = true;

	_boxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = _boxComponent;
	_boxComponent->SetBoxExtent(FVector(36.0f, 8.0f, 8.0f));
	_boxComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	_bodyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshComponent"));
	_bodyMeshComponent->SetupAttachment(_boxComponent);
	_bodyMeshComponent->SetRelativeScale3D(FVector(0.7f, 0.12f, 0.12f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cylinder.Cylinder'"));
	if (MeshFinder.Succeeded())
	{
		_bodyMeshComponent->SetStaticMesh(MeshFinder.Object);
	}

	InitialLifeSpan = 3.0f;
}

void ABullet::BeginPlay()
{
	Super::BeginPlay();

	_boxComponent->OnComponentBeginOverlap.AddDynamic(this, &ABullet::OnHit);
}

void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetActorLocation(GetActorLocation() + GetActorForwardVector() * _speed * DeltaTime, true);
}

void ABullet::SetDamage(float NewDamage)
{
	_damage = NewDamage;
}

void ABullet::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	AEnemy* Enemy = Cast<AEnemy>(OtherActor);
	if (!Enemy)
	{
		return;
	}

	Enemy->ApplyDamage(_damage);
	Destroy();
}
