#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"

UCLASS()
class SHOOTCPP_API ABullet : public AActor
{
	GENERATED_BODY()

public:
	ABullet();

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

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	float _speed = 2600.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	float _damage = 20.0f;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
