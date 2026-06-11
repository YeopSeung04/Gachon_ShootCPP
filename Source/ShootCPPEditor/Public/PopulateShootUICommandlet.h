#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PopulateShootUICommandlet.generated.h"

UCLASS()
class SHOOTCPPEDITOR_API UPopulateShootUICommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UPopulateShootUICommandlet();

	virtual int32 Main(const FString& Params) override;
};
