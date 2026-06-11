#include "PopulateShootUICommandlet.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "WidgetBlueprint.h"
namespace
{
	static UWidgetBlueprint* LoadWidgetBlueprint(const TCHAR* Path)
	{
		return Cast<UWidgetBlueprint>(StaticLoadObject(UWidgetBlueprint::StaticClass(), nullptr, Path));
	}

	static UCanvasPanel* ResetRoot(UWidgetBlueprint* WidgetBlueprint)
	{
		WidgetBlueprint->Modify();
		WidgetBlueprint->WidgetTree->Modify();

		if (WidgetBlueprint->WidgetTree->RootWidget)
		{
			WidgetBlueprint->WidgetTree->RemoveWidget(WidgetBlueprint->WidgetTree->RootWidget);
			WidgetBlueprint->WidgetTree->RootWidget = nullptr;
		}

		UCanvasPanel* Root = WidgetBlueprint->WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetBlueprint->WidgetTree->RootWidget = Root;
		return Root;
	}

	static UCanvasPanelSlot* AddToCanvas(UCanvasPanel* Root, UWidget* Widget, FVector2D Position, FVector2D Size)
	{
		UCanvasPanelSlot* Slot = Root->AddChildToCanvas(Widget);
		Slot->SetPosition(Position);
		Slot->SetSize(Size);
		return Slot;
	}

	static UTextBlock* AddText(UWidgetBlueprint* Blueprint, UCanvasPanel* Root, const TCHAR* Name, const FString& Text, FVector2D Position, FVector2D Size, int32 FontSize, FLinearColor Color = FLinearColor::White, ETextJustify::Type Justification = ETextJustify::Left)
	{
		UTextBlock* TextBlock = Blueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		TextBlock->SetText(FText::FromString(Text));
		TextBlock->SetColorAndOpacity(FSlateColor(Color));
		FSlateFontInfo FontInfo = TextBlock->GetFont();
		FontInfo.Size = FontSize;
		TextBlock->SetFont(FontInfo);
		TextBlock->SetJustification(Justification);
		AddToCanvas(Root, TextBlock, Position, Size);
		return TextBlock;
	}

	static UButton* AddButton(UWidgetBlueprint* Blueprint, UCanvasPanel* Root, const TCHAR* Name, const FString& Text, FVector2D Position, FVector2D Size)
	{
		UButton* Button = Blueprint->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		UTextBlock* Label = Blueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("%s_Text"), Name));
		Label->SetText(FText::FromString(Text));
		Label->SetJustification(ETextJustify::Center);
		Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		FSlateFontInfo FontInfo = Label->GetFont();
		FontInfo.Size = 22;
		Label->SetFont(FontInfo);
		Button->AddChild(Label);
		AddToCanvas(Root, Button, Position, Size);
		return Button;
	}

	static UProgressBar* AddProgress(UWidgetBlueprint* Blueprint, UCanvasPanel* Root, const TCHAR* Name, FVector2D Position, FVector2D Size, float Percent, FLinearColor Color)
	{
		UProgressBar* Bar = Blueprint->WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), Name);
		Bar->SetPercent(Percent);
		Bar->SetFillColorAndOpacity(Color);
		AddToCanvas(Root, Bar, Position, Size);
		return Bar;
	}

	static void Finish(UWidgetBlueprint* Blueprint, TArray<UPackage*>& PackagesToSave)
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		Blueprint->MarkPackageDirty();
		PackagesToSave.AddUnique(Blueprint->GetOutermost());
	}

	static void PopulateLobby(TArray<UPackage*>& PackagesToSave)
	{
		UWidgetBlueprint* Blueprint = LoadWidgetBlueprint(TEXT("/Game/UI/WBP_Lobby.WBP_Lobby"));
		if (!Blueprint) { return; }
		UCanvasPanel* Root = ResetRoot(Blueprint);
		AddText(Blueprint, Root, TEXT("Lobby_TitleText"), TEXT("DUAL ACE: SKY RIFT"), FVector2D(0, 92), FVector2D(1280, 60), 42, FLinearColor::White, ETextJustify::Center);
		AddText(Blueprint, Root, TEXT("Lobby_SubtitleText"), TEXT("C++ 3D Flight Shooting Game"), FVector2D(0, 150), FVector2D(1280, 42), 24, FLinearColor(0.62f, 0.82f, 1.0f, 1.0f), ETextJustify::Center);
		AddButton(Blueprint, Root, TEXT("Lobby_PlayButton"), TEXT("Game Play"), FVector2D(490, 250), FVector2D(300, 52));
		AddButton(Blueprint, Root, TEXT("Lobby_DashboardButton"), TEXT("Dashboard"), FVector2D(490, 316), FVector2D(300, 52));
		AddButton(Blueprint, Root, TEXT("Lobby_ExitButton"), TEXT("Exit"), FVector2D(490, 382), FVector2D(300, 52));
		AddText(Blueprint, Root, TEXT("Lobby_HintText"), TEXT("Edit this screen in WBP_Lobby Designer"), FVector2D(0, 482), FVector2D(1280, 32), 18, FLinearColor(0.76f, 0.8f, 0.88f, 1.0f), ETextJustify::Center);
		Finish(Blueprint, PackagesToSave);
	}

	static void PopulateDashboard(TArray<UPackage*>& PackagesToSave)
	{
		UWidgetBlueprint* Blueprint = LoadWidgetBlueprint(TEXT("/Game/UI/WBP_Dashboard.WBP_Dashboard"));
		if (!Blueprint) { return; }
		UCanvasPanel* Root = ResetRoot(Blueprint);
		AddText(Blueprint, Root, TEXT("Dashboard_TitleText"), TEXT("DASHBOARD"), FVector2D(0, 82), FVector2D(1280, 54), 38, FLinearColor::White, ETextJustify::Center);
		AddText(Blueprint, Root, TEXT("Dashboard_SubtitleText"), TEXT("Local Top 5 leaderboard"), FVector2D(0, 138), FVector2D(1280, 36), 22, FLinearColor(0.62f, 0.82f, 1.0f, 1.0f), ETextJustify::Center);
		AddText(Blueprint, Root, TEXT("Dashboard_LeaderboardTitleText"), TEXT("LEADERBOARD"), FVector2D(370, 220), FVector2D(540, 40), 24);
		AddText(Blueprint, Root, TEXT("Dashboard_LeaderboardText"), TEXT("Records are provided by C++ save data. Redesign this area freely."), FVector2D(370, 278), FVector2D(540, 90), 18, FLinearColor(0.86f, 0.9f, 0.96f, 1.0f));
		AddButton(Blueprint, Root, TEXT("Dashboard_BackButton"), TEXT("Back"), FVector2D(520, 610), FVector2D(240, 46));
		Finish(Blueprint, PackagesToSave);
	}

	static void PopulateShipSelect(TArray<UPackage*>& PackagesToSave)
	{
		UWidgetBlueprint* Blueprint = LoadWidgetBlueprint(TEXT("/Game/UI/WBP_ShipSelect.WBP_ShipSelect"));
		if (!Blueprint) { return; }
		UCanvasPanel* Root = ResetRoot(Blueprint);
		AddText(Blueprint, Root, TEXT("Ship_TitleText"), TEXT("DUAL ACE: SKY RIFT"), FVector2D(0, 70), FVector2D(1280, 58), 40, FLinearColor::White, ETextJustify::Center);
		AddText(Blueprint, Root, TEXT("Ship_SubtitleText"), TEXT("Choose your C++ fighter"), FVector2D(0, 125), FVector2D(1280, 38), 24, FLinearColor(0.62f, 0.82f, 1.0f, 1.0f), ETextJustify::Center);
		AddButton(Blueprint, Root, TEXT("Ship_FalconButton"), TEXT("Falcon\nSpeed 640 / HP 90 / Rapid fire"), FVector2D(284, 190), FVector2D(330, 220));
		AddButton(Blueprint, Root, TEXT("Ship_TitanButton"), TEXT("Titan\nSpeed 500 / HP 170 / Heavy shot"), FVector2D(666, 190), FVector2D(330, 220));
		AddButton(Blueprint, Root, TEXT("Ship_LaunchButton"), TEXT("Launch"), FVector2D(490, 452), FVector2D(300, 52));
		AddButton(Blueprint, Root, TEXT("Ship_BackButton"), TEXT("Back"), FVector2D(520, 516), FVector2D(240, 42));
		AddText(Blueprint, Root, TEXT("Ship_HintText"), TEXT("1 Falcon     2 Titan     Enter Launch"), FVector2D(0, 578), FVector2D(1280, 34), 20, FLinearColor::Yellow, ETextJustify::Center);
		Finish(Blueprint, PackagesToSave);
	}

	static void PopulateCombat(TArray<UPackage*>& PackagesToSave)
	{
		UWidgetBlueprint* Blueprint = LoadWidgetBlueprint(TEXT("/Game/UI/WBP_Combat.WBP_Combat"));
		if (!Blueprint) { return; }
		UCanvasPanel* Root = ResetRoot(Blueprint);
		AddText(Blueprint, Root, TEXT("Combat_TitleText"), TEXT("DUAL ACE"), FVector2D(28, 28), FVector2D(220, 32), 24);
		AddText(Blueprint, Root, TEXT("Combat_HullLabelText"), TEXT("Hull"), FVector2D(28, 48), FVector2D(140, 24), 16);
		AddProgress(Blueprint, Root, TEXT("Combat_HealthBar"), FVector2D(28, 70), FVector2D(270, 18), 1.0f, FLinearColor(0.1f, 0.85f, 0.42f, 1.0f));
		AddText(Blueprint, Root, TEXT("Combat_HealthText"), TEXT("HP 90 / 90"), FVector2D(313, 66), FVector2D(220, 28), 18, FLinearColor(0.8f, 1.0f, 0.86f, 1.0f));
		AddText(Blueprint, Root, TEXT("Combat_ScoreText"), TEXT("Score 0"), FVector2D(28, 102), FVector2D(240, 28), 20);
		AddText(Blueprint, Root, TEXT("Combat_ShipText"), TEXT("Ship Falcon"), FVector2D(28, 128), FVector2D(240, 28), 18, FLinearColor(0.65f, 0.85f, 1.0f, 1.0f));
		AddText(Blueprint, Root, TEXT("Combat_ViewText"), TEXT("View Chase"), FVector2D(28, 152), FVector2D(240, 28), 18, FLinearColor(0.65f, 0.85f, 1.0f, 1.0f));
		AddText(Blueprint, Root, TEXT("Combat_WaveText"), TEXT("Wave 1"), FVector2D(0, 36), FVector2D(1280, 32), 22, FLinearColor::White, ETextJustify::Center);
		AddText(Blueprint, Root, TEXT("Combat_WaveProgressLabelText"), TEXT("Wave Progress"), FVector2D(470, 50), FVector2D(340, 22), 16, FLinearColor::White, ETextJustify::Center);
		AddProgress(Blueprint, Root, TEXT("Combat_WaveProgressBar"), FVector2D(470, 72), FVector2D(340, 10), 0.0f, FLinearColor(0.2f, 0.65f, 1.0f, 1.0f));
		AddProgress(Blueprint, Root, TEXT("Combat_BossHealthBar"), FVector2D(390, 96), FVector2D(500, 20), 0.0f, FLinearColor(0.95f, 0.16f, 0.1f, 1.0f));
		AddText(Blueprint, Root, TEXT("Combat_CrosshairText"), TEXT("+"), FVector2D(615, 330), FVector2D(50, 50), 34, FLinearColor::White, ETextJustify::Center);
		Finish(Blueprint, PackagesToSave);
	}

	static void PopulatePause(TArray<UPackage*>& PackagesToSave)
	{
		UWidgetBlueprint* Blueprint = LoadWidgetBlueprint(TEXT("/Game/UI/WBP_PauseMenu.WBP_PauseMenu"));
		if (!Blueprint) { return; }
		UCanvasPanel* Root = ResetRoot(Blueprint);
		AddText(Blueprint, Root, TEXT("Pause_TitleText"), TEXT("PAUSED"), FVector2D(0, 220), FVector2D(1280, 54), 38, FLinearColor::White, ETextJustify::Center);
		AddText(Blueprint, Root, TEXT("Pause_VolumeText"), TEXT("Master Volume 100%"), FVector2D(0, 300), FVector2D(1280, 36), 24, FLinearColor(0.78f, 0.9f, 1.0f, 1.0f), ETextJustify::Center);
		AddProgress(Blueprint, Root, TEXT("Pause_VolumeBar"), FVector2D(502, 350), FVector2D(276, 18), 1.0f, FLinearColor(0.2f, 0.66f, 1.0f, 1.0f));
		AddButton(Blueprint, Root, TEXT("Pause_VolumeDownButton"), TEXT("-"), FVector2D(406, 390), FVector2D(90, 42));
		AddButton(Blueprint, Root, TEXT("Pause_VolumeUpButton"), TEXT("+"), FVector2D(784, 390), FVector2D(90, 42));
		AddButton(Blueprint, Root, TEXT("Pause_ResumeButton"), TEXT("Resume"), FVector2D(540, 448), FVector2D(200, 42));
		Finish(Blueprint, PackagesToSave);
	}

	static void PopulateGameOver(TArray<UPackage*>& PackagesToSave)
	{
		UWidgetBlueprint* Blueprint = LoadWidgetBlueprint(TEXT("/Game/UI/WBP_GameOver.WBP_GameOver"));
		if (!Blueprint) { return; }
		UCanvasPanel* Root = ResetRoot(Blueprint);
		AddText(Blueprint, Root, TEXT("GameOver_ResultText"), TEXT("MISSION COMPLETE"), FVector2D(0, 120), FVector2D(1280, 58), 40, FLinearColor::Yellow, ETextJustify::Center);
		AddText(Blueprint, Root, TEXT("GameOver_FinalText"), TEXT("Score 0   Wave 1   Ship Falcon   Time 0.0s"), FVector2D(0, 185), FVector2D(1280, 36), 22, FLinearColor::White, ETextJustify::Center);
		AddText(Blueprint, Root, TEXT("GameOver_LeaderboardTitleText"), TEXT("LEADERBOARD"), FVector2D(370, 250), FVector2D(540, 40), 24);
		AddText(Blueprint, Root, TEXT("GameOver_LeaderboardText"), TEXT("Leaderboard data comes from C++ save game."), FVector2D(370, 308), FVector2D(540, 90), 18, FLinearColor(0.86f, 0.9f, 0.96f, 1.0f));
		AddButton(Blueprint, Root, TEXT("GameOver_RestartButton"), TEXT("Restart"), FVector2D(385, 612), FVector2D(230, 46));
		AddButton(Blueprint, Root, TEXT("GameOver_LobbyButton"), TEXT("Lobby"), FVector2D(665, 612), FVector2D(230, 46));
		Finish(Blueprint, PackagesToSave);
	}
}

UPopulateShootUICommandlet::UPopulateShootUICommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UPopulateShootUICommandlet::Main(const FString& Params)
{
	TArray<UPackage*> PackagesToSave;

	PopulateLobby(PackagesToSave);
	PopulateDashboard(PackagesToSave);
	PopulateShipSelect(PackagesToSave);
	PopulateCombat(PackagesToSave);
	PopulatePause(PackagesToSave);
	PopulateGameOver(PackagesToSave);

	UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
	UE_LOG(LogTemp, Display, TEXT("Populated editable WBP UI layouts."));
	return 0;
}
