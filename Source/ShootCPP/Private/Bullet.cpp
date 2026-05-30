// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"

#include "Components/BoxComponent.h"


// Sets default values
ABullet::ABullet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// box 컴포넌트 생성
	boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = boxComp;
	boxComp->SetCollisionProfileName(TEXT("BlockAll"));
	
	
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
void ABullet::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// p = p0 + vt
	
	SetActorLocation(GetActorLocation() + FVector::UpVector * speed * DeltaTime);
}

