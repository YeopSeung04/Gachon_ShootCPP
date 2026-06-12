#include "CPlayer.h"

#include "BigMissile.h"
#include "Bullet.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "ShootGameMode.h"
#include "UObject/ConstructorHelpers.h"

ACPlayer::ACPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	_boxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = _boxComponent;
	_boxComponent->SetBoxExtent(FVector(70.0f, 36.0f, 20.0f));
	_boxComponent->SetCollisionProfileName(TEXT("Pawn"));

	_bodyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshComponent"));
	_bodyMeshComponent->SetupAttachment(_boxComponent);
	_bodyMeshComponent->SetRelativeScale3D(FVector(1.45f, 0.42f, 0.24f));

	_leftWingMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWingMeshComponent"));
	_leftWingMeshComponent->SetupAttachment(_boxComponent);
	_leftWingMeshComponent->SetRelativeLocation(FVector(-14.0f, -58.0f, -4.0f));
	_leftWingMeshComponent->SetRelativeScale3D(FVector(0.65f, 1.25f, 0.08f));

	_rightWingMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWingMeshComponent"));
	_rightWingMeshComponent->SetupAttachment(_boxComponent);
	_rightWingMeshComponent->SetRelativeLocation(FVector(-14.0f, 58.0f, -4.0f));
	_rightWingMeshComponent->SetRelativeScale3D(FVector(0.65f, 1.25f, 0.08f));

	_cockpitMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CockpitMeshComponent"));
	_cockpitMeshComponent->SetupAttachment(_boxComponent);
	_cockpitMeshComponent->SetRelativeLocation(FVector(28.0f, 0.0f, 22.0f));
	_cockpitMeshComponent->SetRelativeScale3D(FVector(0.34f, 0.22f, 0.16f));

	_leftEngineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftEngineMeshComponent"));
	_leftEngineMeshComponent->SetupAttachment(_boxComponent);
	_leftEngineMeshComponent->SetRelativeLocation(FVector(-72.0f, -24.0f, -2.0f));
	_leftEngineMeshComponent->SetRelativeScale3D(FVector(0.24f, 0.18f, 0.18f));

	_rightEngineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightEngineMeshComponent"));
	_rightEngineMeshComponent->SetupAttachment(_boxComponent);
	_rightEngineMeshComponent->SetRelativeLocation(FVector(-72.0f, 24.0f, -2.0f));
	_rightEngineMeshComponent->SetRelativeScale3D(FVector(0.24f, 0.18f, 0.18f));

	_ultimateReadyLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("UltimateReadyLightComponent"));
	_ultimateReadyLightComponent->SetupAttachment(_boxComponent);
	_ultimateReadyLightComponent->SetRelativeLocation(FVector(24.0f, 0.0f, 22.0f));
	_ultimateReadyLightComponent->SetLightColor(FLinearColor(0.0f, 1.0f, 0.95f));
	_ultimateReadyLightComponent->SetIntensity(0.0f);
	_ultimateReadyLightComponent->SetAttenuationRadius(360.0f);
	_ultimateReadyLightComponent->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cone.Cone'"));
	if (MeshFinder.Succeeded())
	{
		_bodyMeshComponent->SetStaticMesh(MeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (CubeMeshFinder.Succeeded())
	{
		_leftWingMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
		_rightWingMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (SphereMeshFinder.Succeeded())
	{
		_cockpitMeshComponent->SetStaticMesh(SphereMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cylinder.Cylinder'"));
	if (CylinderMeshFinder.Succeeded())
	{
		_leftEngineMeshComponent->SetStaticMesh(CylinderMeshFinder.Object);
		_rightEngineMeshComponent->SetStaticMesh(CylinderMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Script/Engine.Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
	if (MaterialFinder.Succeeded())
	{
		_bodyMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_leftWingMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_rightWingMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_cockpitMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_leftEngineMeshComponent->SetMaterial(0, MaterialFinder.Object);
		_rightEngineMeshComponent->SetMaterial(0, MaterialFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundFinder(TEXT("/Script/Engine.SoundWave'/Engine/VREditor/Sounds/UI/Laser_Push.Laser_Push'"));
	if (FireSoundFinder.Succeeded())
	{
		_fireSound = FireSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> CameraSoundFinder(TEXT("/Script/Engine.SoundWave'/Engine/VREditor/Sounds/UI/Click_on_Button.Click_on_Button'"));
	if (CameraSoundFinder.Succeeded())
	{
		_cameraSwitchSound = CameraSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> DamageSoundFinder(TEXT("/Script/Engine.SoundWave'/Engine/VREditor/Sounds/VR_click1.VR_click1'"));
	if (DamageSoundFinder.Succeeded())
	{
		_damageSound = DamageSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> EnemyHitSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/PlayerHit.PlayerHit'"));
	if (EnemyHitSoundFinder.Succeeded())
	{
		_enemyHitSound = EnemyHitSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> WallHitSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/WallHit.WallHit'"));
	if (WallHitSoundFinder.Succeeded())
	{
		_wallHitSound = WallHitSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> LowHealthSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/LowHp.LowHp'"));
	if (LowHealthSoundFinder.Succeeded())
	{
		_lowHealthSound = LowHealthSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> DeathSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/PlayerDeath.PlayerDeath'"));
	if (DeathSoundFinder.Succeeded())
	{
		_deathSound = DeathSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> FirstPersonSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/FirstPersonSwitch.FirstPersonSwitch'"));
	if (FirstPersonSoundFinder.Succeeded())
	{
		_firstPersonSound = FirstPersonSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> ThirdPersonSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/ThirdPersonReturn.ThirdPersonReturn'"));
	if (ThirdPersonSoundFinder.Succeeded())
	{
		_thirdPersonSound = ThirdPersonSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> UltimateFireSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/UltimateFire.UltimateFire'"));
	if (UltimateFireSoundFinder.Succeeded())
	{
		_ultimateFireSound = UltimateFireSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> BankSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Audio/Bank.Bank'"));
	if (BankSoundFinder.Succeeded())
	{
		_bankSound = BankSoundFinder.Object;
	}

	_springArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	_springArmComponent->SetupAttachment(_boxComponent);
	_springArmComponent->bUsePawnControlRotation = false;
	_springArmComponent->bEnableCameraLag = true;
	_springArmComponent->CameraLagSpeed = 8.0f;

	_cameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	_cameraComponent->SetupAttachment(_springArmComponent);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	_bulletClass = ABullet::StaticClass();
	_ultimateMissileClass = ABigMissile::StaticClass();
}

void ACPlayer::BeginPlay()
{
	Super::BeginPlay();

	_health = _maxHealth;
	_shipMaterial = _bodyMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	_cockpitMaterial = _cockpitMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	UpdateUltimateVisuals();
	UpdateCamera();

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	}

	if (PlayerController)
	{
		PlayerController->SetViewTarget(this);
	}
}

void ACPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead())
	{
		return;
	}

	if (!CanPlay())
	{
		return;
	}

	_fireCooldown = FMath::Max(0.0f, _fireCooldown - DeltaTime);
	_wallDamageCooldown = FMath::Max(0.0f, _wallDamageCooldown - DeltaTime);
	_damageLockoutTime = FMath::Max(0.0f, _damageLockoutTime - DeltaTime);

	const float ForwardSpeed = _cruiseSpeed;
	FVector NewLocation = GetActorLocation();
	NewLocation.X += _forwardInput * ForwardSpeed * DeltaTime;
	NewLocation.Y += _rightInput * _strafeSpeed * DeltaTime;
	HandleArenaBounds(NewLocation);
	SetActorLocation(NewLocation, true);

	UpdateBanking(DeltaTime);

	if (_isFiring)
	{
		Fire();
	}
}

void ACPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACPlayer::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACPlayer::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Roll"), this, &ACPlayer::Roll);
	PlayerInputComponent->BindAction(TEXT("ToggleCamera"), IE_Pressed, this, &ACPlayer::ToggleCameraView);
	PlayerInputComponent->BindAction(TEXT("Ultimate"), IE_Pressed, this, &ACPlayer::UseUltimate);
	PlayerInputComponent->BindAction(TEXT("RestartGame"), IE_Pressed, this, &ACPlayer::UseUltimate);
}

void ACPlayer::ApplyDamage(float Damage)
{
	if (!CanReceiveDamage())
	{
		return;
	}

	_health = FMath::Max(0.0f, _health - Damage);
	if (!_hasPlayedLowHealthSound && _health > 0.0f && GetHealthRatio() <= 0.1f)
	{
		_hasPlayedLowHealthSound = true;
		if (_lowHealthSound)
		{
			PlayVoiceSound(_lowHealthSound);
		}
	}

	if (IsDead() && _deathSound)
	{
		PlayVoiceSound(_deathSound);
	}

	if (IsDead())
	{
		AShootGameMode* GameMode = Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
		if (GameMode)
		{
			GameMode->EndGame(false);
		}
	}
}

void ACPlayer::ApplyEnemyDamage(float Damage)
{
	if (!CanReceiveDamage())
	{
		return;
	}

	if (_enemyHitSound && !IsDead())
	{
		PlayVoiceSound(_enemyHitSound);
	}

	ApplyDamage(Damage);
}

void ACPlayer::ApplyWallDamage(float Damage)
{
	if (!CanReceiveDamage())
	{
		return;
	}

	if (_wallHitSound && !IsDead())
	{
		PlayVoiceSound(_wallHitSound);
	}

	ApplyDamage(Damage);
}

void ACPlayer::ApplyShipData(const FPlayerShipData& ShipData)
{
	_shipName = ShipData.DisplayName;
	_shipColor = ShipData.ShipColor;
	_cruiseSpeed = ShipData.CruiseSpeed;
	_strafeSpeed = ShipData.StrafeSpeed;
	_verticalSpeed = ShipData.VerticalSpeed;
	_maxHealth = ShipData.MaxHealth;
	_health = _maxHealth;
	_hasPlayedLowHealthSound = false;
	_bulletDamage = ShipData.BulletDamage;
	_ultimateDamage = ShipData.UltimateDamage;
	_fireInterval = ShipData.FireInterval;
	_wallDamage = ShipData.ShipType == EPlayerShipType::Falcon ? 14.0f : 9.0f;

	const FVector ShipScale = ShipData.ShipType == EPlayerShipType::Falcon
		? FVector(1.45f, 0.48f, 0.24f)
		: FVector(1.8f, 0.78f, 0.4f);
	_bodyMeshComponent->SetRelativeScale3D(ShipScale);

	const float ShipWidth = ShipData.ShipType == EPlayerShipType::Falcon ? 58.0f : 72.0f;
	const float EngineY = ShipData.ShipType == EPlayerShipType::Falcon ? 24.0f : 34.0f;
	_leftWingMeshComponent->SetRelativeLocation(FVector(-14.0f, -ShipWidth, -4.0f));
	_rightWingMeshComponent->SetRelativeLocation(FVector(-14.0f, ShipWidth, -4.0f));
	_leftWingMeshComponent->SetRelativeScale3D(ShipData.ShipType == EPlayerShipType::Falcon ? FVector(0.65f, 1.25f, 0.08f) : FVector(0.85f, 1.55f, 0.12f));
	_rightWingMeshComponent->SetRelativeScale3D(ShipData.ShipType == EPlayerShipType::Falcon ? FVector(0.65f, 1.25f, 0.08f) : FVector(0.85f, 1.55f, 0.12f));
	_leftEngineMeshComponent->SetRelativeLocation(FVector(-72.0f, -EngineY, -2.0f));
	_rightEngineMeshComponent->SetRelativeLocation(FVector(-72.0f, EngineY, -2.0f));

	if (_shipMaterial)
	{
		_shipMaterial->SetVectorParameterValue(TEXT("Color"), _shipColor);
		_shipMaterial->SetVectorParameterValue(TEXT("BaseColor"), _shipColor);
	}

	UpdateUltimateVisuals();
}

void ACPlayer::ResetForGameplayStart()
{
	_health = _maxHealth;
	_score = 0;
	_fireCooldown = 0.0f;
	_wallDamageCooldown = _wallDamageInterval;
	_damageLockoutTime = 1.0f;
	_hasPlayedLowHealthSound = false;
	_ultimateChargeCount = 0;
	ResetInputState();
	UpdateUltimateVisuals();
}

float ACPlayer::GetHealthRatio() const
{
	if (_maxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(_health / _maxHealth, 0.0f, 1.0f);
}

int32 ACPlayer::GetScore() const
{
	return _score;
}

FString ACPlayer::GetShipName() const
{
	return _shipName;
}

bool ACPlayer::IsDead() const
{
	return _health <= 0.0f;
}

bool ACPlayer::IsFirstPersonView() const
{
	return _isFirstPersonView;
}

float ACPlayer::GetHealth() const
{
	return _health;
}

float ACPlayer::GetMaxHealth() const
{
	return _maxHealth;
}

bool ACPlayer::CanReceiveDamage() const
{
	return !IsDead() && CanPlay() && _damageLockoutTime <= 0.0f;
}

void ACPlayer::HealByMaxHealthPercent(float Percent)
{
	if (IsDead() || _maxHealth <= 0.0f || Percent <= 0.0f)
	{
		return;
	}

	_health = FMath::Clamp(_health + _maxHealth * Percent, 0.0f, _maxHealth);
	if (GetHealthRatio() > 0.1f)
	{
		_hasPlayedLowHealthSound = false;
	}
}

void ACPlayer::AddScore(int32 ScoreAmount)
{
	_score += ScoreAmount;
}

void ACPlayer::MoveRight(float Value)
{
	SetMoveRightInput(Value);
}

void ACPlayer::MoveForward(float Value)
{
	SetMoveForwardInput(Value);
}

void ACPlayer::Roll(float Value)
{
	SetRollInput(Value);
}

void ACPlayer::SetMoveRightInput(float Value)
{
	_rightInput = Value;
}

void ACPlayer::SetMoveForwardInput(float Value)
{
	_forwardInput = Value;
}

void ACPlayer::SetRollInput(float Value)
{
	int32 CurrentRollInputSign = 0;
	if (CanPlay())
	{
		if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			const bool IsPressingRightRoll = PlayerController->IsInputKeyDown(EKeys::E);
			const bool IsPressingLeftRoll = PlayerController->IsInputKeyDown(EKeys::Q);
			CurrentRollInputSign = (IsPressingRightRoll ? 1 : 0) - (IsPressingLeftRoll ? 1 : 0);
		}
	}

	if (CurrentRollInputSign != 0 && CurrentRollInputSign != _rollSoundInputSign && _bankSound)
	{
		PlayVoiceSound(_bankSound);
	}

	_rollSoundInputSign = CurrentRollInputSign;
	_wasRolling = CurrentRollInputSign != 0;
	_rollInput = Value;
}

void ACPlayer::ResetInputState()
{
	_rightInput = 0.0f;
	_forwardInput = 0.0f;
	_rollInput = 0.0f;
	_rollSoundInputSign = 0;
	_isFiring = false;
}

void ACPlayer::AddUltimateCharge()
{
	_ultimateChargeCount++;
	UpdateUltimateVisuals();
}

void ACPlayer::SetUltimateChargeCount(int32 ChargeCount)
{
	_ultimateChargeCount = FMath::Max(0, ChargeCount);
	UpdateUltimateVisuals();
}

bool ACPlayer::IsUltimateReady() const
{
	return _ultimateChargeCount > 0;
}

bool ACPlayer::HasUsedUltimateThisWave() const
{
	return _ultimateChargeCount <= 0;
}

bool ACPlayer::CanChargeUltimateThisWave() const
{
	return true;
}

int32 ACPlayer::GetUltimateChargeCount() const
{
	return _ultimateChargeCount;
}

void ACPlayer::StartFire()
{
	if (!CanPlay())
	{
		return;
	}

	_isFiring = true;
	Fire();
}

void ACPlayer::StopFire()
{
	_isFiring = false;
}

void ACPlayer::ToggleCameraView()
{
	if (!CanPlay())
	{
		return;
	}

	_isFirstPersonView = !_isFirstPersonView;
	UpdateCamera();

	USoundBase* ViewSound = _isFirstPersonView ? _firstPersonSound : _thirdPersonSound;
	if (ViewSound)
	{
		PlayVoiceSound(ViewSound);
	}
	else if (_cameraSwitchSound)
	{
		PlayVoiceSound(_cameraSwitchSound);
	}
}

void ACPlayer::SelectFalcon()
{
	AShootGameMode* GameMode = Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->SelectFalcon();
	}
}

void ACPlayer::SelectTitan()
{
	AShootGameMode* GameMode = Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->SelectTitan();
	}
}

void ACPlayer::ConfirmStart()
{
	AShootGameMode* GameMode = Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		if (GameMode->GetGameState() == EShootGameState::Lobby)
		{
			GameMode->OpenShipSelect();
			return;
		}

		GameMode->StartSelectedGame();
	}
}

void ACPlayer::RestartGame()
{
	AShootGameMode* GameMode = Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode && GameMode->IsGameOver())
	{
		GameMode->RestartGame();
	}
}

void ACPlayer::PlayVoiceSound(USoundBase* Sound)
{
	if (!Sound)
	{
		return;
	}

	AShootGameMode* GameMode = Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		if (Sound == _ultimateFireSound)
		{
			GameMode->PlayImportantVoiceSound(Sound);
			return;
		}

		GameMode->PlayVoiceSound(Sound);
		return;
	}

	UGameplayStatics::PlaySound2D(this, Sound, 1.0f);
}

void ACPlayer::UpdateUltimateVisuals()
{
	const FLinearColor ReadyBodyColor(1.0f, 0.86f, 0.1f, 1.0f);
	const FLinearColor ReadyCoreColor(0.0f, 1.0f, 0.95f, 1.0f);
	const bool bHasUltimateCharge = IsUltimateReady();
	const FLinearColor CockpitColor = bHasUltimateCharge ? ReadyCoreColor : FLinearColor(0.06f, 0.08f, 0.1f, 1.0f);
	const FLinearColor BodyColor = bHasUltimateCharge ? ReadyBodyColor : _shipColor;

	if (_shipMaterial)
	{
		_shipMaterial->SetVectorParameterValue(TEXT("Color"), BodyColor);
		_shipMaterial->SetVectorParameterValue(TEXT("BaseColor"), BodyColor);
		_shipMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), bHasUltimateCharge ? ReadyBodyColor * 6.0f : FLinearColor::Black);
	}

	if (_cockpitMaterial)
	{
		_cockpitMaterial->SetVectorParameterValue(TEXT("Color"), CockpitColor);
		_cockpitMaterial->SetVectorParameterValue(TEXT("BaseColor"), CockpitColor);
		_cockpitMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), bHasUltimateCharge ? ReadyCoreColor * 8.0f : FLinearColor::Black);
	}

	if (_ultimateReadyLightComponent)
	{
		_ultimateReadyLightComponent->SetVisibility(bHasUltimateCharge);
		_ultimateReadyLightComponent->SetIntensity(bHasUltimateCharge ? 4200.0f : 0.0f);
	}
}

void ACPlayer::UseUltimate()
{
	AShootGameMode* GameMode = Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode && GameMode->IsGameOver())
	{
		GameMode->RestartGame();
		return;
	}

	if (!CanPlay() || !IsUltimateReady() || !_ultimateMissileClass)
	{
		return;
	}

	_ultimateChargeCount = FMath::Max(0, _ultimateChargeCount - 1);
	UpdateUltimateVisuals();
	if (GameMode)
	{
		GameMode->NotifyPlayerUltimateUsed();
	}

	const FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 180.0f;
	const FRotator SpawnRotation = GetActorRotation();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;

	ABigMissile* Missile = GetWorld()->SpawnActor<ABigMissile>(_ultimateMissileClass, SpawnLocation, SpawnRotation, SpawnParameters);
	if (Missile)
	{
		Missile->SetDamage(_ultimateDamage);
	}

	if (_ultimateFireSound)
	{
		PlayVoiceSound(_ultimateFireSound);
	}
	else if (_fireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, _fireSound, SpawnLocation, 0.9f);
	}
}

void ACPlayer::Fire()
{
	if (!CanPlay() || _fireCooldown > 0.0f || !_bulletClass)
	{
		return;
	}

	_fireCooldown = _fireInterval;

	const FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 115.0f;
	const FRotator SpawnRotation = GetActorRotation();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;

	ABullet* Bullet = GetWorld()->SpawnActor<ABullet>(_bulletClass, SpawnLocation, SpawnRotation, SpawnParameters);
	if (Bullet)
	{
		Bullet->SetDamage(_bulletDamage);
	}

	if (_fireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, _fireSound, SpawnLocation, 0.22f);
	}
}

void ACPlayer::UpdateCamera()
{
	if (_isFirstPersonView)
	{
		_springArmComponent->TargetArmLength = 0.0f;
		_springArmComponent->SocketOffset = FVector(95.0f, 0.0f, 28.0f);
		_springArmComponent->SetRelativeRotation(FRotator::ZeroRotator);
		_cameraComponent->SetFieldOfView(86.0f);
		return;
	}

	_springArmComponent->TargetArmLength = 520.0f;
	_springArmComponent->SocketOffset = FVector(-80.0f, 0.0f, 130.0f);
	_springArmComponent->SetRelativeRotation(FRotator(-7.0f, 0.0f, 0.0f));
	_cameraComponent->SetFieldOfView(72.0f);
}

void ACPlayer::UpdateBanking(float DeltaTime)
{
	const float ManualBank = _rollInput * 48.0f;
	const FRotator TargetRotation(0.0f, 0.0f, ManualBank);
	const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 8.0f);
	SetActorRotation(NewRotation);
}

void ACPlayer::HandleArenaBounds(FVector& Location)
{
	constexpr float MinY = -920.0f;
	constexpr float MaxY = 920.0f;
	constexpr float MinZ = 120.0f;
	constexpr float MaxZ = 920.0f;

	const bool HitWall = Location.Y < MinY || Location.Y > MaxY || Location.Z < MinZ || Location.Z > MaxZ;
	Location.Y = FMath::Clamp(Location.Y, MinY, MaxY);
	Location.Z = FMath::Clamp(Location.Z, MinZ, MaxZ);

	if (HitWall && _wallDamageCooldown <= 0.0f)
	{
		ApplyWallDamage(_wallDamage);
		_wallDamageCooldown = _wallDamageInterval;
	}
}

bool ACPlayer::CanPlay() const
{
	const AShootGameMode* GameMode = Cast<AShootGameMode>(UGameplayStatics::GetGameMode(this));
	return GameMode && GameMode->GetGameState() == EShootGameState::Playing;
}
