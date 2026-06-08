#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemy.generated.h"

UCLASS()
class SHOOTCPP_API AEnemy : public AActor
{
	GENERATED_BODY()

public:
	AEnemy();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	void InitializeEnemy(float Speed, float MaxHealth, float ContactDamage, int32 ScoreValue, const FVector& MeshScale, const FLinearColor& Color);
	virtual void ApplyDamage(float Damage);
	float GetHealthRatio() const;
	float GetHealth() const;
	float GetMaxHealth() const;
	int32 GetScoreValue() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UBoxComponent* _boxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _bodyMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _coreMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _leftBladeMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _rightBladeMeshComponent;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	float _speed = 650.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	float _maxHealth = 40.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	float _contactDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	int32 _scoreValue = 100;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	bool _isBoss = false;

	float _health = 0.0f;

	UPROPERTY()
	class UMaterialInstanceDynamic* _enemyMaterial = nullptr;

	UPROPERTY()
	class USoundBase* _hitSound = nullptr;

	UPROPERTY()
	class USoundBase* _destroySound = nullptr;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void DestroyByPlayer();
};
