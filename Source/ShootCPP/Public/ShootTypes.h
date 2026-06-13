#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ShootTypes.generated.h"

UENUM(BlueprintType)
enum class EShootGameState : uint8
{
	// GameMode가 현재 어떤 화면/상태를 관리하는지 나타낸다.
	// UI 표시, 입력 모드, 스폰 타이머가 이 값에 따라 바뀐다.

	// 시작 메뉴 화면. 게임 시작, 대시보드, 종료 버튼을 보여준다.
	Lobby,

	// 저장된 상위 기록을 확인하는 화면.
	Dashboard,

	// Falcon/Titan 중 사용할 기체를 고르는 화면.
	ShipSelect,

	// 실제 전투가 진행 중인 상태. 이동, 발사, 적 스폰이 활성화된다.
	Playing,

	// 전투 중 일시정지 상태. 게임은 멈추고 볼륨/재개 UI를 보여준다.
	PauseMenu,

	// 승리 또는 패배 후 결과와 리더보드를 보여주는 상태.
	GameOver
};

UENUM(BlueprintType)
enum class EPlayerShipType : uint8
{
	// Falcon은 빠르고 약한 기체, Titan은 느리지만 체력과 화력이 높은 기체다.

	// 빠른 이동과 짧은 연사 간격을 가진 기체.
	Falcon,

	// 느리지만 높은 체력과 강한 단발 공격력을 가진 기체.
	Titan
};

USTRUCT(BlueprintType)
struct FPlayerShipData : public FTableRowBase
{
	GENERATED_BODY()

	// 기체 선택 화면과 실제 전투 능력치가 같은 구조체를 사용한다.
	// 먼저 C++ 코드에 적힌 기본 능력치를 사용하고, Unreal DataTable이 있으면 그 표의 값으로 다시 덮어쓴다.

	// 이 데이터가 Falcon용인지 Titan용인지 구분하는 값.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPlayerShipType ShipType = EPlayerShipType::Falcon;

	// UI와 결과 화면에 표시되는 기체 이름.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName = TEXT("Falcon");

	// 전방/후방 이동에 사용되는 기본 이동 속도.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CruiseSpeed = 620.0f;

	// 좌우 이동에 사용되는 속도. 값이 클수록 회피가 쉬워진다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StrafeSpeed = 1030.0f;

	// 위아래 이동 속도용 값이다.
	// 현재 플레이어 이동 코드는 주로 X/Y 이동을 쓰지만, 기체 데이터에는 확장용으로 포함되어 있다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VerticalSpeed = 860.0f;

	// 최대 체력. 게임 시작과 기체 변경 시 현재 체력도 이 값으로 초기화된다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 90.0f;

	// 일반 탄환 한 발이 적에게 주는 피해량.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BulletDamage = 18.0f;

	// 궁극기 미사일이 적에게 주는 피해량.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UltimateDamage = 210.0f;

	// 일반 공격의 발사 간격. 값이 작을수록 더 빠르게 연사한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireInterval = 0.1f;

	// 기체 Mesh Material에 적용되는 대표 색상.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ShipColor = FLinearColor(0.05f, 0.5f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct FEnemyStatData : public FTableRowBase
{
	GENERATED_BODY()

	// 적 종류별 이동 속도, 체력, 충돌 피해, 점수 보상을 묶어 둔 데이터다.
	// SpawnEnemy에서 Basic/Fast/Tank/Boss 중 하나를 고를 때 사용된다.

	// UI나 디버깅 로그에서 구분하기 위한 적 이름.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName = TEXT("Basic Drone");

	// 적이 플레이어를 향해 접근하는 속도.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 650.0f;

	// 적의 최대 체력. 탄환 피해를 받아 0 이하가 되면 처치된다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 45.0f;

	// 플레이어와 직접 부딪혔을 때 플레이어에게 주는 피해량.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ContactDamage = 18.0f;

	// 이 적을 처치했을 때 플레이어에게 추가되는 점수.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ScoreValue = 100;

	// 적 Mesh의 크기. Basic/Fast/Tank/Boss의 외형 차이를 만드는 데 사용된다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MeshScale = FVector(1.1f, 1.1f, 0.7f);

	// 적 Material에 적용되는 색상.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(0.55f, 0.9f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct FLeaderboardEntry
{
	GENERATED_BODY()

	// 게임 종료 시 저장되는 한 줄의 기록이다.
	// SaveGame 슬롯에 저장되고 Dashboard/GameOver 화면에서 Top 5만 보여준다.

	// 기록에 표시할 플레이어 이름. 현재는 기본 이름 ACE를 사용한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = TEXT("ACE");

	// 기록 달성 시 사용한 기체 이름.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShipName = TEXT("Falcon");

	// 최종 점수. 리더보드는 이 값을 기준으로 먼저 정렬된다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Score = 0;

	// 게임 종료 시점에 도달해 있던 웨이브 번호.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wave = 1;

	// 전투 시작 후 게임이 끝날 때까지 버틴 시간.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurvivedTime = 0.0f;

	// 보스를 잡고 끝났으면 true, 플레이어가 죽어서 끝났으면 false.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DidWin = false;
};

USTRUCT(BlueprintType)
struct FWaveDesign : public FTableRowBase
{
	GENERATED_BODY()

	// 웨이브별 난이도 설계 데이터다.
	// 시간이 지날수록 스폰 간격, 동시 스폰 수, 특수 적 확률을 조정한다.

	// 몇 번째 웨이브인지 나타내는 번호. GameMode가 이 번호 순서대로 웨이브를 진행한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WaveNumber = 1;

	// 적 스폰 사이의 시간 간격. 값이 작을수록 적이 더 자주 나온다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnInterval = 1.35f;

	// 한 번 스폰될 때 동시에 생성되는 적 수.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnemiesPerSpawn = 1;

	// 해당 웨이브에서 기본 적 속도의 기준값.
	// GameMode는 이 값을 기준으로 적 타입별 속도 배율을 계산한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasicEnemySpeed = 650.0f;

	// 해당 웨이브에서 기본 적 체력의 기준값.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasicEnemyHealth = 45.0f;

	// 해당 웨이브에서 기본 적 충돌 피해의 기준값.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasicEnemyDamage = 18.0f;

	// 빠른 적이 나올 확률. 0.42면 약 42% 확률이라는 뜻이다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastEnemyChance = 0.0f;

	// 탱크 적이 나올 확률. GameMode에서는 Tank 판정을 먼저 하고 그 다음 Fast를 판정한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TankEnemyChance = 0.0f;

	// true이면 일반 적 스폰 대신 보스전을 시작하는 웨이브로 처리된다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsBossWave = false;
};
