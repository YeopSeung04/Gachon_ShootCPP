// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemy.generated.h"

UCLASS()
class SHOOTCPP_API AEnemy : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=ShootComponent)
	class UBoxComponent* boxComp;
	UPROPERTY(VisibleAnywhere, Category=ShootComponent)
	class UStaticMeshComponent* bodyMeshComp;
	
public:
	// 필요속성: 이동속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat"); 
	float speed = 1000;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
};
