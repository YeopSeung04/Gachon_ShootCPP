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

	InputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AShootPlayerController::HandlePrimaryPressed);
	InputComponent->BindAction(TEXT("Fire"), IE_Released, this, &AShootPlayerController::HandlePrimaryReleased);
	InputComponent->BindAction(TEXT("SelectFalcon"), IE_Pressed, this, &AShootPlayerController::HandleSelectFalcon);
	InputComponent->BindAction(TEXT("SelectTitan"), IE_Pressed, this, &AShootPlayerController::HandleSelectTitan);
	InputComponent->BindAction(TEXT("ConfirmStart"), IE_Pressed, this, &AShootPlayerController::HandleConfirm);
	InputComponent->BindAction(TEXT("RestartGame"), IE_Pressed, this, &AShootPlayerController::HandleRestart);

	InputComponent->BindAxis(TEXT("MoveRight"), this, &AShootPlayerController::HandleMoveRight);
	InputComponent->BindAxis(TEXT("MoveForward"), this, &AShootPlayerController::HandleMoveForward);
	InputComponent->BindAxis(TEXT("MoveUp"), this, &AShootPlayerController::HandleMoveForward);
	InputComponent->BindAxis(TEXT("Roll"), this, &AShootPlayerController::HandleRoll);
	InputComponent->BindAxis(TEXT("Throttle"), this, &AShootPlayerController::HandleRoll);
}

void AShootPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

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

void AShootPlayerController::HandlePrimaryPressed()
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (!GameMode)
	{
		return;
	}

	if (GameMode->GetGameState() == EShootGameState::Playing)
	{
		ACPlayer* ShootPlayer = GetShootPlayer();
		if (ShootPlayer)
		{
			ShootPlayer->StartFire();
		}
		return;
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
	if (GameMode && GameMode->IsGameOver())
	{
		GameMode->RestartGame();
	}
}

void AShootPlayerController::HandleMoveRight(float Value)
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (!GameMode || GameMode->GetGameState() != EShootGameState::Playing)
	{
		return;
	}

	ACPlayer* ShootPlayer = GetShootPlayer();
	if (ShootPlayer)
	{
		ShootPlayer->SetMoveRightInput(Value);
	}
}

void AShootPlayerController::HandleMoveForward(float Value)
{
	AShootGameMode* GameMode = GetShootGameMode();
	if (!GameMode || GameMode->GetGameState() != EShootGameState::Playing)
	{
		return;
	}

	ACPlayer* ShootPlayer = GetShootPlayer();
	if (ShootPlayer)
	{
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
	if (!GameMode || GameMode->GetGameState() != EShootGameState::Playing)
	{
		ShootPlayer->SetMoveRightInput(0.0f);
		ShootPlayer->SetMoveForwardInput(0.0f);
		ShootPlayer->SetRollInput(0.0f);
		return;
	}

	const float RightInput = (IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f) - (IsInputKeyDown(EKeys::A) ? 1.0f : 0.0f);
	const float ForwardInput = (IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f) - (IsInputKeyDown(EKeys::S) ? 1.0f : 0.0f);
	const float RollInput = (IsInputKeyDown(EKeys::E) ? 1.0f : 0.0f) - (IsInputKeyDown(EKeys::Q) ? 1.0f : 0.0f);

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
