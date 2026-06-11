#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CreateShootDataTablesCommandlet.generated.h"

UCLASS()
class SHOOTCPPEDITOR_API UCreateShootDataTablesCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UCreateShootDataTablesCommandlet();

	virtual int32 Main(const FString& Params) override;
};
