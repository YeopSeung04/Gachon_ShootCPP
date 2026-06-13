#include "SpaceArena.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	// 맵에 배치된 장식 Actor를 태그로 찾아 재활용한다.
	// 태그 이름을 한 곳에 모아 두면 레벨 배치와 C++ 코드의 연결점을 확인하기 쉽다.
	const FName FloorSegmentTag(TEXT("ArenaFloorSegment"));
	const FName LeftRailTag(TEXT("ArenaLeftRail"));
	const FName RightRailTag(TEXT("ArenaRightRail"));
	const FName GateTopTag(TEXT("ArenaGateTop"));
	const FName GateLeftTag(TEXT("ArenaGateLeft"));
	const FName GateRightTag(TEXT("ArenaGateRight"));
	const FName StarTag(TEXT("ArenaStar"));
}

ASpaceArena::ASpaceArena()
{
	PrimaryActorTick.bCanEverTick = true;

	_rootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = _rootComponent;
}

void ASpaceArena::BeginPlay()
{
	Super::BeginPlay();

	// 레벨에 미리 배치한 바닥/레일/게이트/별을 찾아 배열에 저장한다.
	CachePlacedArenaActors();

	// 각 오브젝트의 Material을 동적으로 만들어 색상을 코드에서 통일한다.
	for (AActor* FloorSegment : _floorSegments)
	{
		ApplyColor(FloorSegment, FLinearColor(0.015f, 0.08f, 0.12f, 1.0f));
	}

	for (AActor* Rail : _leftRails)
	{
		ApplyColor(Rail, FLinearColor(0.0f, 0.45f, 1.0f, 1.0f));
	}

	for (AActor* Rail : _rightRails)
	{
		ApplyColor(Rail, FLinearColor(0.0f, 0.45f, 1.0f, 1.0f));
	}

	for (AActor* GateBar : _gateTopBars)
	{
		ApplyColor(GateBar, FLinearColor(0.95f, 0.82f, 0.16f, 1.0f));
	}

	for (AActor* GateBar : _gateLeftBars)
	{
		ApplyColor(GateBar, FLinearColor(0.95f, 0.82f, 0.16f, 1.0f));
	}

	for (AActor* GateBar : _gateRightBars)
	{
		ApplyColor(GateBar, FLinearColor(0.95f, 0.82f, 0.16f, 1.0f));
	}

	for (AActor* Star : _stars)
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

	// 플레이어 이동에 맞춰 뒤로 지나간 장식물을 앞으로 보내 무한히 이어지는 우주 통로처럼 보이게 한다.
	const float FloorTotalLength = FloorSegmentCount * FloorSegmentSpacing;
	const float GateTotalLength = GateCount * GateSpacing;

	for (AActor* FloorSegment : _floorSegments)
	{
		RecycleLinearActor(FloorSegment, PlayerX, FloorTotalLength);
	}

	for (AActor* Rail : _leftRails)
	{
		RecycleLinearActor(Rail, PlayerX, FloorTotalLength);
	}

	for (AActor* Rail : _rightRails)
	{
		RecycleLinearActor(Rail, PlayerX, FloorTotalLength);
	}

	for (AActor* GateBar : _gateTopBars)
	{
		RecycleLinearActor(GateBar, PlayerX, GateTotalLength);
	}

	for (AActor* GateBar : _gateLeftBars)
	{
		RecycleLinearActor(GateBar, PlayerX, GateTotalLength);
	}

	for (AActor* GateBar : _gateRightBars)
	{
		RecycleLinearActor(GateBar, PlayerX, GateTotalLength);
	}

	for (AActor* Star : _stars)
	{
		RecycleLinearActor(Star, PlayerX, FloorTotalLength);
	}
}

void ASpaceArena::CachePlacedArenaActors()
{
	CacheActorsWithTag(FloorSegmentTag, _floorSegments);
	CacheActorsWithTag(LeftRailTag, _leftRails);
	CacheActorsWithTag(RightRailTag, _rightRails);
	CacheActorsWithTag(GateTopTag, _gateTopBars);
	CacheActorsWithTag(GateLeftTag, _gateLeftBars);
	CacheActorsWithTag(GateRightTag, _gateRightBars);
	CacheActorsWithTag(StarTag, _stars);
}

void ASpaceArena::CacheActorsWithTag(FName Tag, TArray<AActor*>& OutActors) const
{
	OutActors.Empty();
	UGameplayStatics::GetAllActorsWithTag(this, Tag, OutActors);
	// 이름순 정렬은 재활용 순서를 안정적으로 만들어 디버깅하기 쉽게 한다.
	OutActors.Sort([](const AActor& Left, const AActor& Right)
	{
		return Left.GetName() < Right.GetName();
	});
}

void ASpaceArena::ApplyColor(AActor* Actor, const FLinearColor& Color)
{
	if (!Actor)
	{
		return;
	}

	UStaticMeshComponent* MeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>();
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

void ASpaceArena::RecycleLinearActor(AActor* Actor, float PlayerX, float TotalLength)
{
	if (!Actor)
	{
		return;
	}

	FVector Location = Actor->GetActorLocation();
	const float RecycleDistance = TotalLength * 0.5f;

	if (Location.X < PlayerX - RecycleDistance)
	{
		// 플레이어 뒤쪽 반 바퀴 밖으로 벗어나면 전체 길이만큼 앞으로 이동시킨다.
		do
		{
			Location.X += TotalLength;
		}
		while (Location.X < PlayerX - RecycleDistance);

		Actor->SetActorLocation(Location);
		return;
	}

	if (Location.X > PlayerX + RecycleDistance)
	{
		// 반대로 너무 앞에 있는 경우도 뒤로 감아 배치한다.
		do
		{
			Location.X -= TotalLength;
		}
		while (Location.X > PlayerX + RecycleDistance);

		Actor->SetActorLocation(Location);
	}
}
