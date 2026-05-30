// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CPlayer.generated.h"

// 사용자 입력에 따라 상하좌우 이동하게 하고 싶다.

UCLASS()
class SHOOTCPP_API ACPlayer : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=ShootComponent)
	class UBoxComponent* boxComp;
	UPROPERTY(VisibleAnywhere, Category=ShootComponent)
	class UStaticMeshComponent* bodyMeshComp;
	
public:
	// 필요속성: 이동속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat"); 
	float speed = 500;
	
public:
	// 사용자 입력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input");
	class UInputAction* ia_move;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input");
	class UInputMappingContext* imc_shoot;
	
	void MovePlayer( const struct FInputActionValue& value);
	FVector direction;
};
