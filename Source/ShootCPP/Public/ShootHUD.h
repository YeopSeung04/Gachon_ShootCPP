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

private:
	void DrawLobby(class AShootGameMode* GameMode);
	void DrawDashboard(class AShootGameMode* GameMode);
	void DrawShipSelect(class AShootGameMode* GameMode);
	void DrawCombatHud(class AShootGameMode* GameMode, class ACPlayer* Player);
	void DrawGameOver(class AShootGameMode* GameMode, class ACPlayer* Player);
	void DrawEnemyHealthBars();
	void DrawLeaderboard(const TArray<FLeaderboardEntry>& Entries, float X, float Y);
	void DrawShipCard(float X, float Y, float Width, float Height, const FPlayerShipData& ShipData, bool IsSelected, FName HitBoxName);
	void DrawButton(float X, float Y, float Width, float Height, const FString& Text, const FLinearColor& FillColor, FName HitBoxName);
	void HandleLobbyClick(class AShootGameMode* GameMode, FName BoxName);
	void HandleDashboardClick(class AShootGameMode* GameMode, FName BoxName);
	void HandleShipSelectClick(class AShootGameMode* GameMode, FName BoxName);
	void HandleGameOverClick(class AShootGameMode* GameMode, FName BoxName);
	void DrawBar(float X, float Y, float Width, float Height, float Ratio, const FLinearColor& FillColor, const FString& Label);
	void DrawCenteredText(const FString& Text, float Y, const FLinearColor& Color, UFont* Font, float Scale);
};
