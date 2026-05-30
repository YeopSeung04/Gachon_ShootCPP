// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "Components/BoxComponent.h"


// Sets default values
AEnemy::AEnemy()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// box 컴포넌트 생성
	boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = boxComp;
	boxComp->SetCollisionProfileName(TEXT("OverlapAll"));
	
	bodyMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshComp"));
	bodyMeshComp->SetupAttachment(boxComp);
	
	// bodymesh 에 cube static mesh 데이터 로드해서 할당하기 
	ConstructorHelpers::FObjectFinder<UStaticMesh> tempMesh(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (tempMesh.Succeeded())
	{
		bodyMeshComp->SetStaticMesh(tempMesh.Object);
	}
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	boxComp->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnHit);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SetActorLocation(GetActorLocation() + FVector::DownVector * speed * DeltaTime);
}

void AEnemy::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	OtherActor->Destroy();
	Destroy();
}

