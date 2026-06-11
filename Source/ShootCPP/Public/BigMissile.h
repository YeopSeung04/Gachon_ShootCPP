#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BigMissile.generated.h"

UCLASS()
class SHOOTCPP_API ABigMissile : public AActor
{
	GENERATED_BODY()

public:
	ABigMissile();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	void SetDamage(float NewDamage);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UBoxComponent* _boxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _bodyMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _coreMeshComponent;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	float _speed = 1850.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	float _damage = 210.0f;

	TSet<TWeakObjectPtr<class AEnemy>> _damagedEnemies;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
