#include "ShootGameInstance.h"

void UShootGameInstance::SetSelectedShipType(EPlayerShipType ShipType)
{
	_selectedShipType = ShipType;
}

EPlayerShipType UShootGameInstance::GetSelectedShipType() const
{
	return _selectedShipType;
}
