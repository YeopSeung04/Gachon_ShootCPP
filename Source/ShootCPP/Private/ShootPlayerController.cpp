#include "ShootPlayerController.h"

#include "CPlayer.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "ShootGameMode.h"

void AShootPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ApplyMenuInputMode();
}

void AShootPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 입력 이름은 Config/DefaultInput.ini에 정의되어 있고, 여기서 전투/메뉴 동작으로 연결
	InputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AShootPlayerController::HandlePrimaryPressed);
	InputComponent->BindAction(TEXT("Fire"), IE_Released, this, &AShootPlayerController::HandlePrimaryReleased);
	InputComponent->BindAction(TEXT("SelectFalcon"), IE_Pressed, this, &AShootPlayerController::HandleSelectFalcon);
	InputComponent->BindAction(TEXT("SelectTitan"), IE_Pressed, this, &AShootPlayerController::HandleSelectTitan);
	InputComponent->BindAction(TEXT("ConfirmStart"), IE_Pressed, this, &AShootPlayerController::HandleConfirm);
	InputComponent->BindAction(TEXT("RestartGame"), IE_Pressed, this, &AShootPlayerController::HandleRestart);
	FInputActionBinding& PauseBinding = InputComponent->BindAction(TEXT("PauseMenu"), IE_Pressed, this, &AShootPlayerController::HandlePauseMenu);
	// 게임이 일시정지되면 일반 입력은 멈추므로, Pause 키만 예외적으로 계속 받음
	PauseBinding.bExecuteWhenPaused = true;

	InputComponent->BindAxis(TEXT("MoveRight"), this, &AShootPlayerController::HandleMoveRight);
	InputComponent->BindAxis(TEXT("MoveForward"), this, &AShootPlayerController::HandleMoveForward);
	InputComponent->BindAxis(TEXT("MoveUp"), this, &AShootPlayerController::HandleMoveForward);
	InputComponent->BindAxis(TEXT("Roll"), this, &AShootPlayerController::HandleRoll);
	InputComponent->BindAxis(TEXT("Throttle"), this, &AShootPlayerController::HandleRoll);
}

void AShootPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// PlayerController는 매 프레임 키보드 상태를 확인해서
	// "현재 어느 방향으로 움직이려는지"를 Player에게 전달한다.
	PollMovementKeys();
}

void AShootPlayerController::ApplyMenuInputMode()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AShootPlayerController::ApplyGameInputMode()
{
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	SetInputMode(FInputModeGameOnly());
	FlushPressedKeys();
}

void AShootPlayerController::ApplyPauseInputMode()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AShootPlayerController::HandlePrimaryPressed()
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (!GameMode)
	{
		return;
	}

	if (GameMode->GetGameState() == EShootGameState::Playing)
	{
		// 전투 중 좌클릭은 발사 입력으로 사용한다.
		ACPlayer* ShootPlayer = GetShootPlayer();
		if (ShootPlayer)
		{
			ShootPlayer->StartFire();
		}
		return;
	}

	float ScreenX = 0.0f;
	float ScreenY = 0.0f;
	if (GetMousePosition(ScreenX, ScreenY))
	{
		// 메뉴 상태에서는 같은 좌클릭을 버튼 클릭 좌표로 넘긴다.
		GameMode->HandlePrimaryClick(ScreenX, ScreenY);
	}
}

void AShootPlayerController::HandlePrimaryReleased()
{
	ACPlayer* ShootPlayer = GetShootPlayer();
	if (ShootPlayer)
	{
		ShootPlayer->StopFire();
	}
}

void AShootPlayerController::HandleSelectFalcon()
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (GameMode)
	{
		GameMode->SelectFalcon();
	}
}

void AShootPlayerController::HandleSelectTitan()
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (GameMode)
	{
		GameMode->SelectTitan();
	}
}

void AShootPlayerController::HandleConfirm()
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (!GameMode)
	{
		return;
	}

	if (GameMode->GetGameState() == EShootGameState::Lobby)
	{
		GameMode->OpenShipSelect();
		return;
	}

	GameMode->StartSelectedGame();
}

void AShootPlayerController::HandleRestart()
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (!GameMode)
	{
		return;
	}

	if (GameMode->IsGameOver())
	{
		GameMode->RestartGame();
		return;
	}

	if (GameMode->GetGameState() == EShootGameState::Playing)
	{
		ACPlayer* ShootPlayer = GetShootPlayer();
		if (ShootPlayer)
		{
			ShootPlayer->UseUltimate();
		}
	}
}

void AShootPlayerController::HandlePauseMenu()
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (GameMode)
	{
		GameMode->TogglePauseMenu();
	}
}

void AShootPlayerController::HandleMoveRight(float Value)
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (!GameMode || GameMode->GetGameState() != EShootGameState::Playing)
	{
		// 전투 중이 아닐 때는 Axis 입력이 들어와도 플레이어 이동값에 반영하지 않는다.
		return;
	}

	ACPlayer* ShootPlayer = GetShootPlayer();
	if (ShootPlayer)
	{
		// Project Settings의 MoveRight Axis 값이 들어오는 경우 Player에 저장한다.
		// 실제 위치 이동은 CPlayer::Tick에서 처리한다.
		ShootPlayer->SetMoveRightInput(Value);
	}
}

void AShootPlayerController::HandleMoveForward(float Value)
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (!GameMode || GameMode->GetGameState() != EShootGameState::Playing)
	{
		// 전투 상태가 아니면 전후 이동값을 무시한다.
		return;
	}

	ACPlayer* ShootPlayer = GetShootPlayer();
	if (ShootPlayer)
	{
		// Project Settings의 MoveForward Axis 값이 들어오는 경우 Player에 저장한다.
		// 저장된 값은 _forwardInput이 되고, CPlayer::Tick에서 X 위치 이동에 사용된다.
		ShootPlayer->SetMoveForwardInput(Value);
	}
}

void AShootPlayerController::HandleRoll(float Value)
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (!GameMode || GameMode->GetGameState() != EShootGameState::Playing)
	{
		return;
	}

	ACPlayer* ShootPlayer = GetShootPlayer();
	if (ShootPlayer)
	{
		ShootPlayer->SetRollInput(Value);
	}
}

void AShootPlayerController::PollMovementKeys()
{
	ACPlayer* ShootPlayer = GetShootPlayer();
	if (!ShootPlayer)
	{
		return;
	}

	AShootGameMode* GameMode = GetShootGameMode();
	if (!GameMode || GameMode->GetGameState() != EShootGameState::Playing || GameMode->IsPauseMenuOpen())
	{
		// 메뉴/일시정지 상태로 바뀐 순간에도 마지막 이동 입력이 남지 않도록 강제로 0
		ShootPlayer->SetMoveRightInput(0.0f);
		ShootPlayer->SetMoveForwardInput(0.0f);
		ShootPlayer->SetRollInput(0.0f);
		return;
	}

	// Axis 이벤트가 환경에 따라 누락되어도 WASD/QE 키 상태를 매 프레임 읽어 안정적으로 조작
	// D는 +1, A는 -1이므로 둘 다 안 누르면 0, 둘 다 누르면 서로 상쇄되어 0이 된다.
	const float RightInput = (IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f) - (IsInputKeyDown(EKeys::A) ? 1.0f : 0.0f);

	// W는 +1, S는 -1로 계산한다.
	// 이 값은 CPlayer에서 X축 이동량을 계산할 때 사용된다.
	const float ForwardInput = (IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f) - (IsInputKeyDown(EKeys::S) ? 1.0f : 0.0f);

	// E는 오른쪽 롤, Q는 왼쪽 롤이다.
	// 위치 이동보다는 기체 기울기와 뱅킹 효과음에 사용된다.
	const float RollInput = (IsInputKeyDown(EKeys::E) ? 1.0f : 0.0f) - (IsInputKeyDown(EKeys::Q) ? 1.0f : 0.0f);

	// 여기서는 이동을 직접 하지 않고 입력값만 Player에게 넘긴다.
	// 실제 위치 변경은 Player Pawn의 Tick에서 한 번에 처리한다.
	ShootPlayer->SetMoveRightInput(RightInput);
	ShootPlayer->SetMoveForwardInput(ForwardInput);
	ShootPlayer->SetRollInput(RollInput);
}

AShootGameMode* AShootPlayerController::GetShootGameMode() const
{
	return Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
}

ACPlayer* AShootPlayerController::GetShootPlayer() const
{
	return Cast<ACPlayer>(GetPawn());
}
