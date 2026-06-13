#include "Bullet.h"

#include "Components/BoxComponent.h"
#include "Enemy.h"

ABullet::ABullet()
{
	PrimaryActorTick.bCanEverTick = true;

	// 탄환은 충돌 박스를 Root로 사용한다.
	// 메시보다 충돌 범위를 명확히 관리하기 쉽고, Overlap 이벤트로 적중을 처리한다.
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

	// 월드 밖으로 멀리 날아간 탄환이 계속 남지 않도록 자동 삭제 시간을 둔다.
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

	// Forward 방향으로 계속 이동한다. DeltaTime을 곱해 FPS가 달라도 속도가 일정하다.
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

	// 일반 탄환은 한 번 맞으면 피해를 주고 즉시 사라진다.
	Enemy->ApplyDamage(_damage);
	Destroy();
}
