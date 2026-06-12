#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ShootTypes.h"
#include "ShootGameInstance.generated.h"

UCLASS()
class SHOOTCPP_API UShootGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	void SetSelectedShipType(EPlayerShipType ShipType);
	EPlayerShipType GetSelectedShipType() const;

private:
	EPlayerShipType _selectedShipType = EPlayerShipType::Falcon;
};
