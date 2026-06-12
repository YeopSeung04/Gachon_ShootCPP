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
		Tracking,
		Charging,
		Dashing,
		Retreating
	};

	FVector _startLocation = FVector::ZeroVector;
	FVector _dashTargetLocation = FVector::ZeroVector;
	FVector _retreatTargetLocation = FVector::ZeroVector;
	float _phase = 0.0f;
	float _stateTime = 0.0f;
	float _nextChargeTime = 3.2f;
	EBossAttackState _attackState = EBossAttackState::Tracking;

	UPROPERTY()
	class UMaterialInstanceDynamic* _reactorCoreMaterial = nullptr;

	void UpdateAttackState(float DeltaTime, const FVector& PlayerLocation);
	void BeginCharge(const FVector& PlayerLocation);
	void BeginDash(const FVector& PlayerLocation);
	void BeginRetreat(const FVector& PlayerLocation);
	void MoveToward(const FVector& TargetLocation, float MoveSpeed, float DeltaTime);
	void UpdateChargeVisual(float ChargeRatio);
};
