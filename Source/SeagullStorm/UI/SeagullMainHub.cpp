#include "UI/SeagullMainHub.h"
#include "UI/SeagullGiftCodePanel.h"
#include "UI/SeagullFeedbackForm.h"
#include "UI/SeagullSettingsPanel.h"
#include "UI/SeagullWidgetStyles.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameMode.h"
#include "Core/SeagullTypes.h"
#include "Audio/SeagullAudioManager.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Data/SeagullConfigCache.h"
#include "Data/SeagullSaveData.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "SeagullStorm.h"

void USeagullMainHub::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* Background = SeagullWidgetStyles::MakePanel(WidgetTree, SeagullColors::DarkBG, 0.f);
	if (UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(Background))
	{
		BackgroundSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		BackgroundSlot->SetOffsets(FMargin(0.f));
	}

	// Full-screen column with an outer margin
	UVerticalBox* Screen = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	if (UCanvasPanelSlot* ScreenSlot = RootCanvas->AddChildToCanvas(Screen))
	{
		ScreenSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		ScreenSlot->SetOffsets(FMargin(40.f));
	}

	// --- Header: title | coins | best ---
	UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UTextBlock* HubTitle = SeagullWidgetStyles::MakeText(WidgetTree, TEXT("Seagull Storm"),
		SeagullWidgetStyles::TextFontSize, SeagullColors::Orange);
	if (UHorizontalBoxSlot* TitleSlot = Header->AddChildToHorizontalBox(HubTitle))
	{
		TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TitleSlot->SetVerticalAlignment(VAlign_Center);
	}
	CoinsText = SeagullWidgetStyles::MakeText(WidgetTree, TEXT("Coins: 0"),
		SeagullWidgetStyles::TextFontSize, SeagullColors::XPGold, TEXT("CoinsText"));
	if (UHorizontalBoxSlot* CoinsSlot = Header->AddChildToHorizontalBox(CoinsText))
	{
		CoinsSlot->SetPadding(FMargin(24.f, 0.f, 0.f, 0.f));
		CoinsSlot->SetVerticalAlignment(VAlign_Center);
	}
	HighscoreText = SeagullWidgetStyles::MakeText(WidgetTree, TEXT("Best: 0"),
		SeagullWidgetStyles::TextFontSize, SeagullColors::LightText, TEXT("HighscoreText"));
	if (UHorizontalBoxSlot* BestSlot = Header->AddChildToHorizontalBox(HighscoreText))
	{
		BestSlot->SetPadding(FMargin(24.f, 0.f, 0.f, 0.f));
		BestSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UVerticalBoxSlot* HeaderSlot = Screen->AddChildToVerticalBox(Header))
	{
		HeaderSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));
	}

	// --- Body: upgrades (left) | play/leaderboard/news (right) ---
	UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	if (UVerticalBoxSlot* BodySlot = Screen->AddChildToVerticalBox(Body))
	{
		BodySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	// Left column: upgrade panel
	UBorder* LeftPanel = SeagullWidgetStyles::MakePanel(WidgetTree, SeagullColors::PanelBG, 20.f);
	if (UHorizontalBoxSlot* LeftSlot = Body->AddChildToHorizontalBox(LeftPanel))
	{
		FSlateChildSize LeftSize(ESlateSizeRule::Fill);
		LeftSize.Value = 0.38f;
		LeftSlot->SetSize(LeftSize);
		LeftSlot->SetPadding(FMargin(0.f, 0.f, 20.f, 0.f));
	}
	UVerticalBox* UpgradeColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	LeftPanel->AddChild(UpgradeColumn);

	UTextBlock* UpgradesHeader = SeagullWidgetStyles::MakeText(WidgetTree, TEXT("UPGRADES"),
		SeagullWidgetStyles::TextFontSize, SeagullColors::Orange);
	SeagullWidgetStyles::AddRow(UpgradeColumn, UpgradesHeader, 6.f);

	SeagullWidgetStyles::AddRow(UpgradeColumn, BuildUpgradeRow(SpeedLevelText, BuySpeedButton, TEXT("Speed")));
	SeagullWidgetStyles::AddRow(UpgradeColumn, BuildUpgradeRow(DamageLevelText, BuyDamageButton, TEXT("Damage")));
	SeagullWidgetStyles::AddRow(UpgradeColumn, BuildUpgradeRow(HPLevelText, BuyHPButton, TEXT("HP")));
	SeagullWidgetStyles::AddRow(UpgradeColumn, BuildUpgradeRow(MagnetLevelText, BuyMagnetButton, TEXT("Magnet")));

	// Right column
	UVerticalBox* RightColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	if (UHorizontalBoxSlot* RightSlot = Body->AddChildToHorizontalBox(RightColumn))
	{
		FSlateChildSize RightSize(ESlateSizeRule::Fill);
		RightSize.Value = 0.62f;
		RightSlot->SetSize(RightSize);
	}

	PlayButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("PLAY"),
		SeagullColors::Orange, SeagullColors::DarkText, 32, TEXT("PlayButton"));
	SeagullWidgetStyles::AddRow(RightColumn, PlayButton, 6.f);

	UTextBlock* LeaderboardHeader = SeagullWidgetStyles::MakeText(WidgetTree, TEXT("LEADERBOARD"),
		SeagullWidgetStyles::TextFontSize, SeagullColors::Orange);
	SeagullWidgetStyles::AddRow(RightColumn, LeaderboardHeader, 6.f);

	UScrollBox* LeaderboardScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
	LeaderboardBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LeaderboardBox"));
	LeaderboardScroll->AddChild(LeaderboardBox);
	if (UVerticalBoxSlot* ScrollSlot = RightColumn->AddChildToVerticalBox(LeaderboardScroll))
	{
		ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		ScrollSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
	}

	UTextBlock* NewsHeader = SeagullWidgetStyles::MakeText(WidgetTree, TEXT("NEWS"),
		SeagullWidgetStyles::TextFontSize, SeagullColors::Orange);
	SeagullWidgetStyles::AddRow(RightColumn, NewsHeader, 6.f);

	NewsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NewsBox"));
	SeagullWidgetStyles::AddRow(RightColumn, NewsBox, 4.f);

	// Bottom action buttons
	UHorizontalBox* ActionRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	GiftCodeButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Gift Code"),
		SeagullColors::Teal, SeagullColors::LightText, SeagullWidgetStyles::SmallFontSize, TEXT("GiftCodeButton"));
	FeedbackButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Feedback"),
		SeagullColors::Teal, SeagullColors::LightText, SeagullWidgetStyles::SmallFontSize, TEXT("FeedbackButton"));
	SettingsButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Settings"),
		SeagullColors::Teal, SeagullColors::LightText, SeagullWidgetStyles::SmallFontSize, TEXT("SettingsButton"));
	UButton* ActionButtons[] = { GiftCodeButton, FeedbackButton, SettingsButton };
	for (UButton* ActionButton : ActionButtons)
	{
		if (UHorizontalBoxSlot* ActionSlot = ActionRow->AddChildToHorizontalBox(ActionButton))
		{
			ActionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			ActionSlot->SetPadding(FMargin(4.f, 0.f));
		}
	}
	SeagullWidgetStyles::AddRow(RightColumn, ActionRow, 8.f);
}

UWidget* USeagullMainHub::BuildUpgradeRow(UTextBlock*& OutLevelText, UButton*& OutBuyButton, const FString& RowName)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

	OutLevelText = SeagullWidgetStyles::MakeText(WidgetTree, FString::Printf(TEXT("%s Lv.0"), *RowName),
		SeagullWidgetStyles::SmallFontSize, SeagullColors::LightText, FName(*FString::Printf(TEXT("%sLevelText"), *RowName)));
	OutLevelText->SetAutoWrapText(true);
	if (UHorizontalBoxSlot* TextSlot = Row->AddChildToHorizontalBox(OutLevelText))
	{
		TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TextSlot->SetVerticalAlignment(VAlign_Center);
	}

	OutBuyButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("+"),
		SeagullColors::Orange, SeagullColors::DarkText, SeagullWidgetStyles::SmallFontSize,
		FName(*FString::Printf(TEXT("Buy%sButton"), *RowName)));
	if (UHorizontalBoxSlot* ButtonSlot = Row->AddChildToHorizontalBox(OutBuyButton))
	{
		ButtonSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));
		ButtonSlot->SetVerticalAlignment(VAlign_Center);
	}

	return Row;
}

void USeagullMainHub::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayButton) PlayButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnPlayClicked);
	if (GiftCodeButton) GiftCodeButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnGiftCodeClicked);
	if (FeedbackButton) FeedbackButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnFeedbackClicked);
	if (SettingsButton) SettingsButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnSettingsClicked);
	if (BuySpeedButton) BuySpeedButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnBuySpeed);
	if (BuyDamageButton) BuyDamageButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnBuyDamage);
	if (BuyHPButton) BuyHPButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnBuyHP);
	if (BuyMagnetButton) BuyMagnetButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnBuyMagnet);

	LoadHubData();
}

void USeagullMainHub::LoadHubData()
{
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	USeagullHorizonManager* HM = GI->GetHorizonManager();
	if (!HM) return;

	// Weak self for every async SDK callback: the hub widget can be removed and
	// GC'd (PLAY / sign-out) before an HTTP response lands. GI and HM live on the
	// GameInstance and stay valid for the whole session.
	TWeakObjectPtr<USeagullMainHub> WeakThis(this);

	// 1. Load Remote Config
	if (!GI->bConfigLoaded)
	{
		HM->LoadAllConfigs([WeakThis, GI, HM](bool bSuccess, const TMap<FString, FString>& Configs)
		{
			if (bSuccess)
			{
				GI->GetConfigCache()->ParseFromConfigs(Configs);
				GI->bConfigLoaded = true;
				UE_LOG(LogSeagullStorm, Log, TEXT("Remote Config loaded: %d keys"), Configs.Num());
				if (USeagullMainHub* Hub = WeakThis.Get())
				{
					Hub->RefreshDisplay();
				}
			}
			else
			{
				if (HM)
				{
					HM->RecordException(
						TEXT("Remote Config load failed"),
						TEXT("LoadAllConfigs returned bSuccess=false"));
				}
			}
		});
	}

	// 2. Load Cloud Save
	if (!GI->bSaveLoaded)
	{
		HM->LoadData([WeakThis, GI, HM](bool bSuccess, const FString& Data)
		{
			if (bSuccess && !Data.IsEmpty())
			{
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Data);
				if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
				{
					GI->SaveData = FSeagullSaveData::FromJsonString(Data);
					GI->bSaveLoaded = true;
					UE_LOG(LogSeagullStorm, Log, TEXT("Cloud Save loaded: %d coins"), GI->SaveData.Coins);
				}
				else
				{
					UE_LOG(LogSeagullStorm, Warning, TEXT("Cloud Save JSON parse failed, using defaults"));
					if (HM)
					{
						HM->RecordException(
							TEXT("Cloud save JSON deserialization failed"),
							FString::Printf(TEXT("Data length: %d, snippet: %s"), Data.Len(), *Data.Left(200)));
					}
					GI->SaveData.InitDefaults();
					GI->bSaveLoaded = true;
				}
			}
			else
			{
				GI->SaveData.InitDefaults();
				GI->bSaveLoaded = true;
			}
			if (USeagullMainHub* Hub = WeakThis.Get())
			{
				Hub->RefreshDisplay();
			}
		});
	}

	// 3. Load Leaderboard Top 10
	HM->GetTop(10, [WeakThis, HM](bool bSuccess, const TArray<FHorizonLeaderboardEntry>& Entries)
	{
		USeagullMainHub* Hub = WeakThis.Get();
		if (bSuccess && Hub && Hub->LeaderboardBox)
		{
			Hub->LeaderboardBox->ClearChildren();
			for (const FHorizonLeaderboardEntry& Entry : Entries)
			{
				UTextBlock* Row = NewObject<UTextBlock>(Hub);
				Row->SetText(FText::FromString(
					FString::Printf(TEXT("#%d  %s  %lld"), Entry.Position, *Entry.Username, Entry.Score)));
				Row->SetFont(SeagullWidgetStyles::GetPixelFont(SeagullWidgetStyles::SmallFontSize));
				Row->SetColorAndOpacity(FSlateColor(SeagullColors::LightText));
				Hub->LeaderboardBox->AddChildToVerticalBox(Row);
			}
			UE_LOG(LogSeagullStorm, Log, TEXT("Leaderboard loaded: %d entries"), Entries.Num());
		}
		else if (!bSuccess && HM)
		{
			HM->RecordException(
				TEXT("Leaderboard GetTop failed"),
				TEXT("GetTop returned bSuccess=false"));
		}
	});

	// 4. Load News (cache in GameInstance for pause menu reuse)
	if (!GI->bNewsLoaded)
	{
		HM->LoadNews(5, TEXT("en"), [WeakThis, GI](bool bSuccess, const TArray<FHorizonNewsEntry>& Entries)
		{
			if (bSuccess)
			{
				GI->CachedNews = Entries;
				GI->bNewsLoaded = true;
			}
			if (USeagullMainHub* Hub = WeakThis.Get())
			{
				Hub->DisplayNews(GI->CachedNews);
			}
		});
	}
	else
	{
		DisplayNews(GI->CachedNews);
	}

	RefreshDisplay();
}

void USeagullMainHub::RefreshDisplay()
{
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	if (CoinsText)
		CoinsText->SetText(FText::FromString(FString::Printf(TEXT("Coins: %d"), GI->SaveData.Coins)));
	if (HighscoreText)
		HighscoreText->SetText(FText::FromString(FString::Printf(TEXT("Best: %d"), GI->SaveData.Highscore)));

	UpdateUpgradeRow(SpeedLevelText, TEXT("Speed"), TEXT("speed"));
	UpdateUpgradeRow(DamageLevelText, TEXT("Damage"), TEXT("damage"));
	UpdateUpgradeRow(HPLevelText, TEXT("HP"), TEXT("hp"));
	UpdateUpgradeRow(MagnetLevelText, TEXT("Magnet"), TEXT("magnet"));
}

void USeagullMainHub::UpdateUpgradeRow(UTextBlock* LevelText, const FString& Label, const FString& Key)
{
	if (!LevelText) return;

	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	const int32 Level = GI->SaveData.GetUpgradeLevel(Key);
	FString Suffix;
	if (USeagullConfigCache* Config = GI->GetConfigCache())
	{
		if (Level >= Config->GetUpgradeMax(Key))
		{
			Suffix = TEXT("  MAX");
		}
		else
		{
			Suffix = FString::Printf(TEXT("  (%d)"), Config->GetUpgradeCost(Key, Level));
		}
	}
	LevelText->SetText(FText::FromString(FString::Printf(TEXT("%s Lv.%d%s"), *Label, Level, *Suffix)));
}

void USeagullMainHub::OnPlayClicked()
{
	ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		GM->StartRun();
	}
}

void USeagullMainHub::OnGiftCodeClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	USeagullGiftCodePanel* Panel = CreateWidget<USeagullGiftCodePanel>(PC);
	if (Panel) Panel->AddToViewport(100);
}

void USeagullMainHub::OnFeedbackClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	USeagullFeedbackForm* Form = CreateWidget<USeagullFeedbackForm>(PC);
	if (Form) Form->AddToViewport(100);
}

void USeagullMainHub::OnSettingsClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	USeagullSettingsPanel* Panel = CreateWidget<USeagullSettingsPanel>(PC);
	if (Panel) Panel->AddToViewport(100);
}

void USeagullMainHub::OnBuySpeed() { TryBuyUpgrade(TEXT("speed")); }
void USeagullMainHub::OnBuyDamage() { TryBuyUpgrade(TEXT("damage")); }
void USeagullMainHub::OnBuyHP() { TryBuyUpgrade(TEXT("hp")); }
void USeagullMainHub::OnBuyMagnet() { TryBuyUpgrade(TEXT("magnet")); }

void USeagullMainHub::TryBuyUpgrade(const FString& Key)
{
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI || !GI->GetConfigCache()) return;

	int32 CurrentLevel = GI->SaveData.GetUpgradeLevel(Key);
	int32 MaxLevel = GI->GetConfigCache()->GetUpgradeMax(Key);

	if (CurrentLevel >= MaxLevel)
	{
		UE_LOG(LogSeagullStorm, Log, TEXT("Upgrade %s already at max level"), *Key);
		return;
	}

	int32 Cost = GI->GetConfigCache()->GetUpgradeCost(Key, CurrentLevel);
	if (GI->SaveData.Coins < Cost)
	{
		UE_LOG(LogSeagullStorm, Log, TEXT("Not enough coins for %s upgrade (need %d, have %d)"), *Key, Cost, GI->SaveData.Coins);
		return;
	}

	GI->SaveData.Coins -= Cost;
	GI->SaveData.Upgrades.FindOrAdd(Key) = CurrentLevel + 1;

	// Record breadcrumb for crash reporting
	USeagullHorizonManager* HM = GI->GetHorizonManager();
	if (HM)
	{
		int32 NewLevel = CurrentLevel + 1;
		HM->RecordBreadcrumb(TEXT("user_action"), FString::Printf(TEXT("bought_%s_%d"), *Key, NewLevel));
	}

	UE_LOG(LogSeagullStorm, Log, TEXT("Bought %s upgrade: level %d -> %d (cost %d)"), *Key, CurrentLevel, CurrentLevel + 1, Cost);

	// Play upgrade SFX
	ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM && GM->AudioManager && GM->AudioManager->SFX_UpgradeSelect)
	{
		GM->AudioManager->PlaySFX(GM->AudioManager->SFX_UpgradeSelect, GetWorld());
	}

	RefreshDisplay();
}

void USeagullMainHub::DisplayNews(const TArray<FHorizonNewsEntry>& Entries)
{
	if (!NewsBox) return;
	NewsBox->ClearChildren();
	for (const FHorizonNewsEntry& Entry : Entries)
	{
		UTextBlock* Row = NewObject<UTextBlock>(this);
		Row->SetText(FText::FromString(
			FString::Printf(TEXT("[%s] %s"), *Entry.ReleaseDate, *Entry.Title)));
		Row->SetFont(SeagullWidgetStyles::GetPixelFont(SeagullWidgetStyles::SmallFontSize));
		Row->SetColorAndOpacity(FSlateColor(SeagullColors::LightText));
		NewsBox->AddChildToVerticalBox(Row);
	}
	UE_LOG(LogSeagullStorm, Log, TEXT("News displayed: %d entries"), Entries.Num());
}
