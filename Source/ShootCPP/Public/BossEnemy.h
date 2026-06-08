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

	FVector _startLocation = FVector::ZeroVector;
	float _phase = 0.0f;
};
