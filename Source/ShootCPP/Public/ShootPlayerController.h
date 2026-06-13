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
	void ApplyPauseInputMode();

private:
	// Controller는 키/마우스 입력을 해석해 Player와 GameMode에 전달한다.
	// 실제 이동/발사 계산은 Player가, 메뉴 상태 변경은 GameMode가 수행한다.
	void HandlePrimaryPressed();
	void HandlePrimaryReleased();
	void HandleSelectFalcon();
	void HandleSelectTitan();
	void HandleConfirm();
	void HandleRestart();
	void HandlePauseMenu();
	void HandleMoveRight(float Value);
	void HandleMoveForward(float Value);
	void HandleRoll(float Value);
	void PollMovementKeys();

	class AShootGameMode* GetShootGameMode() const;
	class ACPlayer* GetShootPlayer() const;
};
