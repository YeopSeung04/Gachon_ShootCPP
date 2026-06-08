#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ShootTypes.h"
#include "LeaderboardSaveGame.generated.h"

UCLASS()
class SHOOTCPP_API ULeaderboardSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FLeaderboardEntry> Entries;
};
