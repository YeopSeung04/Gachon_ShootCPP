#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShootPlayerController.generated.h"

UCLASS()
class SHOOTCPP_API AShootPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	void ApplyMenuInputMode();
	void ApplyGameInputMode();

private:
	void HandlePrimaryPressed();
	void HandlePrimaryReleased();
	void HandleSelectFalcon();
	void HandleSelectTitan();
	void HandleConfirm();
	void HandleRestart();
	void HandleMoveRight(float Value);
	void HandleMoveForward(float Value);
	void HandleRoll(float Value);
	void PollMovementKeys();

	class AShootGameMode* GetShootGameMode() const;
	class ACPlayer* GetShootPlayer() const;
};
