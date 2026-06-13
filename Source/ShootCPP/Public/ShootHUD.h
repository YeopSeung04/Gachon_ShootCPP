#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShootTypes.h"
#include "ShootHUD.generated.h"

UCLASS()
class SHOOTCPP_API AShootHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	virtual void NotifyHitBoxClick(FName BoxName) override;

protected:
	// UMG 위젯이 있으면 해당 위젯을 화면에 올리고,
	// 없으면 아래 Draw 함수들이 Canvas 기반 fallback UI를 직접 그린다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shoot|UMG")
	TSubclassOf<class UShootUserWidget> LobbyWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shoot|UMG")
	TSubclassOf<class UShootUserWidget> DashboardWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shoot|UMG")
	TSubclassOf<class UShootUserWidget> ShipSelectWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shoot|UMG")
	TSubclassOf<class UShootUserWidget> CombatWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shoot|UMG")
	TSubclassOf<class UShootUserWidget> PauseMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shoot|UMG")
	TSubclassOf<class UShootUserWidget> GameOverWidgetClass;

private:
	UPROPERTY()
	class UShootUserWidget* _activeWidget = nullptr;

	EShootGameState _activeWidgetState = EShootGameState::Lobby;

	void SyncWidgetForState(class AShootGameMode* GameMode);
	TSubclassOf<class UShootUserWidget> GetWidgetClassForState(EShootGameState GameState) const;
	bool IsUsingWidgetForState(EShootGameState GameState) const;
	void DrawLobby(class AShootGameMode* GameMode);
	void DrawDashboard(class AShootGameMode* GameMode);
	void DrawShipSelect(class AShootGameMode* GameMode);
	void DrawCombatHud(class AShootGameMode* GameMode, class ACPlayer* Player);
	void DrawUltimateStatus(class ACPlayer* Player);
	void DrawPauseMenu(class AShootGameMode* GameMode);
	void DrawGameOver(class AShootGameMode* GameMode, class ACPlayer* Player);
	void DrawEnemyHealthBars();
	void DrawLeaderboard(const TArray<FLeaderboardEntry>& Entries, float X, float Y);
	void DrawShipCard(float X, float Y, float Width, float Height, const FPlayerShipData& ShipData, bool IsSelected, FName HitBoxName);
	void DrawButton(float X, float Y, float Width, float Height, const FString& Text, const FLinearColor& FillColor, FName HitBoxName);
	void HandleLobbyClick(class AShootGameMode* GameMode, FName BoxName);
	void HandleDashboardClick(class AShootGameMode* GameMode, FName BoxName);
	void HandleShipSelectClick(class AShootGameMode* GameMode, FName BoxName);
	void HandlePauseClick(class AShootGameMode* GameMode, FName BoxName);
	void HandleGameOverClick(class AShootGameMode* GameMode, FName BoxName);
	void DrawBar(float X, float Y, float Width, float Height, float Ratio, const FLinearColor& FillColor, const FString& Label);
	void DrawCenteredText(const FString& Text, float Y, const FLinearColor& Color, UFont* Font, float Scale);
};
