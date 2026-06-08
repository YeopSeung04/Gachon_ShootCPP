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
	void ApplyShipData(const FPlayerShipData& ShipData);
	void StartFire();
	void StopFire();
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
	class USpringArmComponent* _springArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shoot|Component", meta=(AllowPrivateAccess="true"))
	class UCameraComponent* _cameraComponent;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Combat")
	TSubclassOf<class ABullet> _bulletClass;

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
	float _fireInterval = 0.14f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Combat")
	float _wallDamage = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category="Shoot|Combat")
	float _wallDamageInterval = 0.45f;

	UPROPERTY()
	class UMaterialInstanceDynamic* _shipMaterial = nullptr;

	UPROPERTY()
	class USoundBase* _fireSound = nullptr;

	UPROPERTY()
	class USoundBase* _cameraSwitchSound = nullptr;

	UPROPERTY()
	class USoundBase* _damageSound = nullptr;

	float _health = 0.0f;
	float _rightInput = 0.0f;
	float _forwardInput = 0.0f;
	float _rollInput = 0.0f;
	float _fireCooldown = 0.0f;
	float _wallDamageCooldown = 0.0f;
	int32 _score = 0;
	FString _shipName = TEXT("Falcon");
	bool _isFirstPersonView = false;
	bool _isFiring = false;

	void MoveRight(float Value);
	void MoveForward(float Value);
	void Roll(float Value);
	void ToggleCameraView();
	void SelectFalcon();
	void SelectTitan();
	void ConfirmStart();
	void RestartGame();
	void Fire();
	void UpdateCamera();
	void UpdateBanking(float DeltaTime);
	void HandleArenaBounds(FVector& Location);
	bool CanPlay() const;
};
