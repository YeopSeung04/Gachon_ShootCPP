#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CreateShootMapsCommandlet.generated.h"

UCLASS()
class SHOOTCPPEDITOR_API UCreateShootMapsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UCreateShootMapsCommandlet();

	virtual int32 Main(const FString& Params) override;
};
