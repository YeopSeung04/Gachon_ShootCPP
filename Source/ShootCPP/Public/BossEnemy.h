#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "BossEnemy.generated.h"

UCLASS()
class SHOOTCPP_API ABossEnemy : public AEnemy
{
	GENERATED_BODY()

public:
	ABossEnemy();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	// 보스는 Enemy의 체력/피해 시스템을 그대로 쓰고,
	// 아래 상태 머신 값들로 추적 -> 충전 -> 돌진 -> 복귀 패턴을 반복한다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _carrierDeckMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _leftHangarMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _rightHangarMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _reactorCoreMeshComponent;

	enum class EBossAttackState
	{
		// 플레이어 앞쪽을 따라다니며 다음 돌진 시간을 기다리는 상태.
		Tracking,

		// 돌진 직전 예고 상태. 코어가 커지고 색이 바뀌어 플레이어에게 위험을 알려준다.
		Charging,

		// 정해진 목표 지점으로 빠르게 돌진하는 실제 공격 상태.
		Dashing,

		// 돌진 후 다시 플레이어 앞쪽으로 빠져나와 다음 패턴을 준비하는 상태.
		Retreating
	};

	// 보스가 처음 Spawn된 위치. 현재 패턴 확장용으로 보관한다.
	FVector _startLocation = FVector::ZeroVector;

	// Dashing 상태에서 보스가 향해 갈 목표 위치.
	FVector _dashTargetLocation = FVector::ZeroVector;

	// Retreating 상태에서 보스가 돌아갈 목표 위치.
	FVector _retreatTargetLocation = FVector::ZeroVector;

	// Sin 이동을 만들기 위한 누적 시간 값. 보스가 완전히 직선으로만 움직이지 않게 한다.
	float _phase = 0.0f;

	// 현재 상태에 들어온 뒤 지난 시간. Charging/Dashing/Retreating 지속 시간 계산에 사용한다.
	float _stateTime = 0.0f;

	// Tracking 상태에서 다음 Charging으로 넘어가기까지 남은 시간.
	float _nextChargeTime = 3.2f;

	// 현재 보스 공격 상태.
	EBossAttackState _attackState = EBossAttackState::Tracking;

	// 차징 코어 색상/발광을 런타임에 바꾸기 위한 동적 Material.
	UPROPERTY()
	class UMaterialInstanceDynamic* _reactorCoreMaterial = nullptr;

	void UpdateAttackState(float DeltaTime, const FVector& PlayerLocation);
	void BeginCharge(const FVector& PlayerLocation);
	void BeginDash(const FVector& PlayerLocation);
	void BeginRetreat(const FVector& PlayerLocation);
	void MoveToward(const FVector& TargetLocation, float MoveSpeed, float DeltaTime);
	void UpdateChargeVisual(float ChargeRatio);
};
