#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceArena.generated.h"

UCLASS()
class SHOOTCPP_API ASpaceArena : public AActor
{
	GENERATED_BODY()

public:
	ASpaceArena();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	static constexpr int32 FloorSegmentCount = 14;
	static constexpr int32 GateCount = 10;
	static constexpr int32 StarCount = 28;
	static constexpr float FloorSegmentSpacing = 950.0f;
	static constexpr float GateSpacing = 1450.0f;

	UPROPERTY(VisibleAnywhere, Category="Arena")
	class USceneComponent* _rootComponent;

	UPROPERTY()
	TArray<class AActor*> _floorSegments;

	UPROPERTY()
	TArray<class AActor*> _leftRails;

	UPROPERTY()
	TArray<class AActor*> _rightRails;

	UPROPERTY()
	TArray<class AActor*> _gateTopBars;

	UPROPERTY()
	TArray<class AActor*> _gateLeftBars;

	UPROPERTY()
	TArray<class AActor*> _gateRightBars;

	UPROPERTY()
	TArray<class AActor*> _stars;

	void CachePlacedArenaActors();
	void CacheActorsWithTag(FName Tag, TArray<class AActor*>& OutActors) const;
	void ApplyColor(class AActor* Actor, const FLinearColor& Color);
	void RecycleLinearActor(class AActor* Actor, float PlayerX, float TotalLength);
};
