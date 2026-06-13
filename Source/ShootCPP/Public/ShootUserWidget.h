#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShootTypes.h"
#include "ShootUserWidget.generated.h"

UCLASS(Blueprintable)
class SHOOTCPP_API UShootUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// UMG Blueprint에서 버튼을 누르거나 텍스트/바 값을 읽을 때 호출되는 C++ 연결 클래스다.
	// UI는 이 클래스를 통해 GameMode와 Player의 현재 상태를 가져온다.
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Shoot|UI")
	void OpenLobby();

	UFUNCTION(BlueprintCallable, Category="Shoot|UI")
	void OpenDashboard();

	UFUNCTION(BlueprintCallable, Category="Shoot|UI")
	void OpenShipSelect();

	UFUNCTION(BlueprintCallable, Category="Shoot|UI")
	void SelectFalcon();

	UFUNCTION(BlueprintCallable, Category="Shoot|UI")
	void SelectTitan();

	UFUNCTION(BlueprintCallable, Category="Shoot|UI")
	void StartSelectedGame();

	UFUNCTION(BlueprintCallable, Category="Shoot|UI")
	void RestartGame();

	UFUNCTION(BlueprintCallable, Category="Shoot|UI")
	void QuitGame();

	UFUNCTION(BlueprintCallable, Category="Shoot|UI")
	void ClosePauseMenu();

	UFUNCTION(BlueprintCallable, Category="Shoot|UI")
	void IncreaseMasterVolume();

	UFUNCTION(BlueprintCallable, Category="Shoot|UI")
	void DecreaseMasterVolume();

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	EShootGameState GetShootGameState() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	float GetPlayerHealthRatio() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	float GetPlayerHealth() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	float GetPlayerMaxHealth() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	int32 GetPlayerScore() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	FString GetPlayerShipName() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	int32 GetCurrentWave() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	float GetWaveProgressRatio() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	float GetBossHealthRatio() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	float GetMasterVolume() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	bool IsBossActive() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	bool DidPlayerWin() const;

	UFUNCTION(BlueprintPure, Category="Shoot|UI")
	const TArray<FLeaderboardEntry>& GetLeaderboardEntries() const;

private:
	void UpdateRuntimeText();
	FText BuildLeaderboardText(const TArray<FLeaderboardEntry>& Entries) const;
	void SetTextByName(FName WidgetName, const FText& Text);
	void SetProgressByName(FName WidgetName, float Percent);
	void SetVisibilityByName(FName WidgetName, ESlateVisibility NewVisibility);
	class AShootGameMode* GetShootGameMode() const;
	class ACPlayer* GetShootPlayer() const;
};
