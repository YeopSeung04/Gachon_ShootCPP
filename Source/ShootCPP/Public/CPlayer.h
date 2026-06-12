#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShootTypes.h"
#include "CPlayer.generated.h"

UCLASS()
class SHOOTCPP_API ACPlayer : public APawn
{
	GENERATED_BODY()

public:
	ACPlayer();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void ApplyDamage(float Damage);
	void ApplyEnemyDamage(float Damage);
	void ApplyWallDamage(float Damage);
	void ApplyShipData(const FPlayerShipData& ShipData);
	void ResetForGameplayStart();
	void StartFire();
	void StopFire();
	void UseUltimate();
	void AddUltimateCharge();
	void SetUltimateChargeCount(int32 ChargeCount);
	bool IsUltimateReady() const;
	bool HasUsedUltimateThisWave() const;
	bool CanChargeUltimateThisWave() const;
	int32 GetUltimateChargeCount() const;
	void SetMoveRightInput(float Value);
	void SetMoveForwardInput(float Value);
	void SetRollInput(float Value);
	void ResetInputState();
	float GetHealthRatio() const;
	int32 GetScore() const;
	FString GetShipName() const;
	bool IsDead() const;
	bool IsFirstPersonView() const;
	float GetHealth() const;
	float GetMaxHealth() const;
	bool CanReceiveDamage() const;
	void HealByMaxHealthPercent(float Percent);
	void AddScore(int32 ScoreAmount);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UBoxComponent* _boxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _bodyMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _leftWingMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _rightWingMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _cockpitMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _leftEngineMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UStaticMeshComponent* _rightEngineMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UPointLightComponent* _ultimateReadyLightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class USpringArmComponent* _springArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UCameraComponent* _cameraComponent;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Combat")
	TSubclassOf<class ABullet> _bulletClass;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Combat")
	TSubclassOf<class ABigMissile> _ultimateMissileClass;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	float _cruiseSpeed = 520.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	float _strafeSpeed = 900.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	float _verticalSpeed = 760.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Stat")
	float _maxHealth = 120.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Combat")
	float _bulletDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Combat")
	float _ultimateDamage = 210.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Combat")
	float _fireInterval = 0.14f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Combat")
	float _wallDamage = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Combat")
	float _wallDamageInterval = 0.45f;

	UPROPERTY()
	class UMaterialInstanceDynamic* _shipMaterial = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* _cockpitMaterial = nullptr;

	FLinearColor _shipColor = FLinearColor(0.05f, 0.5f, 1.0f, 1.0f);

	UPROPERTY()
	class USoundBase* _fireSound = nullptr;

	UPROPERTY()
	class USoundBase* _cameraSwitchSound = nullptr;

	UPROPERTY()
	class USoundBase* _damageSound = nullptr;

	UPROPERTY()
	class USoundBase* _enemyHitSound = nullptr;

	UPROPERTY()
	class USoundBase* _wallHitSound = nullptr;

	UPROPERTY()
	class USoundBase* _lowHealthSound = nullptr;

	UPROPERTY()
	class USoundBase* _deathSound = nullptr;

	UPROPERTY()
	class USoundBase* _firstPersonSound = nullptr;

	UPROPERTY()
	class USoundBase* _thirdPersonSound = nullptr;

	UPROPERTY()
	class USoundBase* _ultimateFireSound = nullptr;

	UPROPERTY()
	class USoundBase* _bankSound = nullptr;

	float _health = 0.0f;
	float _rightInput = 0.0f;
	float _forwardInput = 0.0f;
	float _rollInput = 0.0f;
	float _fireCooldown = 0.0f;
	float _wallDamageCooldown = 0.0f;
	float _damageLockoutTime = 0.0f;
	int32 _score = 0;
	FString _shipName = TEXT("Falcon");
	bool _isFirstPersonView = false;
	bool _isFiring = false;
	int32 _ultimateChargeCount = 0;
	bool _hasPlayedLowHealthSound = false;
	bool _wasRolling = false;
	int32 _rollSoundInputSign = 0;

	void MoveRight(float Value);
	void MoveForward(float Value);
	void Roll(float Value);
	void ToggleCameraView();
	void SelectFalcon();
	void SelectTitan();
	void ConfirmStart();
	void RestartGame();
	void Fire();
	void PlayVoiceSound(class USoundBase* Sound);
	void UpdateUltimateVisuals();
	void UpdateCamera();
	void UpdateBanking(float DeltaTime);
	void HandleArenaBounds(FVector& Location);
	bool CanPlay() const;
};
