#include "UI/SeagullMainHub.h"
#include "UI/SeagullGiftCodePanel.h"
#include "UI/SeagullFeedbackForm.h"
#include "UI/SeagullSettingsPanel.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameMode.h"
#include "Audio/SeagullAudioManager.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Data/SeagullConfigCache.h"
#include "Data/SeagullSaveData.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "SeagullStorm.h"

void USeagullMainHub::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayButton) PlayButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnPlayClicked);
	if (GiftCodeButton) GiftCodeButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnGiftCodeClicked);
	if (FeedbackButton) FeedbackButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnFeedbackClicked);
	if (SettingsButton) SettingsButton->OnClicked.AddDynamic(this, &USeagullMainHub::OnSettingsClicked);

	LoadHubData();
}

void USeagullMainHub::LoadHubData()
{
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	USeagullHorizonManager* HM = GI->GetHorizonManager();
	if (!HM) return;

	// 1. Load Remote Config
	if (!GI->bConfigLoaded)
	{
		HM->LoadAllConfigs([GI, HM](bool bSuccess, const TMap<FString, FString>& Configs)
		{
			if (bSuccess)
			{
				GI->GetConfigCache()->ParseFromConfigs(Configs);
				GI->bConfigLoaded = true;
				UE_LOG(LogSeagullStorm, Log, TEXT("Remote Config loaded: %d keys"), Configs.Num());
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
		HM->LoadData([GI, HM](bool bSuccess, const FString& Data)
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
		});
	}

	// 3. Load Leaderboard Top 10
	HM->GetTop(10, [this, HM](bool bSuccess, const TArray<FHorizonLeaderboardEntry>& Entries)
	{
		if (bSuccess && LeaderboardBox)
		{
			LeaderboardBox->ClearChildren();
			for (const FHorizonLeaderboardEntry& Entry : Entries)
			{
				UTextBlock* Row = NewObject<UTextBlock>(this);
				Row->SetText(FText::FromString(
					FString::Printf(TEXT("#%d  %s  %lld"), Entry.Position, *Entry.Username, Entry.Score)));
				LeaderboardBox->AddChildToVerticalBox(Row);
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
		HM->LoadNews(5, TEXT("en"), [this, GI](bool bSuccess, const TArray<FHorizonNewsEntry>& Entries)
		{
			if (bSuccess)
			{
				GI->CachedNews = Entries;
				GI->bNewsLoaded = true;
			}
			DisplayNews(GI->CachedNews);
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
		NewsBox->AddChildToVerticalBox(Row);
	}
	UE_LOG(LogSeagullStorm, Log, TEXT("News displayed: %d entries"), Entries.Num());
}
