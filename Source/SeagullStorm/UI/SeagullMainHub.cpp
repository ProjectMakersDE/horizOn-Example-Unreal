#include "UI/SeagullMainHub.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameMode.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Data/SeagullConfigCache.h"
#include "Data/SeagullSaveData.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
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
		HM->LoadAllConfigs([GI](bool bSuccess, const TMap<FString, FString>& Configs)
		{
			if (bSuccess)
			{
				GI->GetConfigCache()->ParseFromConfigs(Configs);
				GI->bConfigLoaded = true;
				UE_LOG(LogSeagullStorm, Log, TEXT("Remote Config loaded: %d keys"), Configs.Num());
			}
		});
	}

	// 2. Load Cloud Save
	if (!GI->bSaveLoaded)
	{
		HM->LoadData([GI](bool bSuccess, const FString& Data)
		{
			if (bSuccess && !Data.IsEmpty())
			{
				GI->SaveData = FSeagullSaveData::FromJsonString(Data);
				GI->bSaveLoaded = true;
				UE_LOG(LogSeagullStorm, Log, TEXT("Cloud Save loaded: %d coins"), GI->SaveData.Coins);
			}
			else
			{
				GI->SaveData.InitDefaults();
				GI->bSaveLoaded = true;
			}
		});
	}

	// 3. Load Leaderboard Top 10
	HM->GetTop(10, [](bool bSuccess, const TArray<FHorizonLeaderboardEntry>& Entries)
	{
		if (bSuccess)
		{
			UE_LOG(LogSeagullStorm, Log, TEXT("Leaderboard loaded: %d entries"), Entries.Num());
		}
	});

	// 4. Load News
	HM->LoadNews(5, TEXT("en"), [](bool bSuccess, const TArray<FHorizonNewsEntry>& Entries)
	{
		if (bSuccess)
		{
			UE_LOG(LogSeagullStorm, Log, TEXT("News loaded: %d entries"), Entries.Num());
		}
	});

	// 5. Start Crash Capture
	HM->StartCrashCapture();

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
	UE_LOG(LogSeagullStorm, Log, TEXT("Gift Code panel requested"));
}

void USeagullMainHub::OnFeedbackClicked()
{
	UE_LOG(LogSeagullStorm, Log, TEXT("Feedback form requested"));
}

void USeagullMainHub::OnSettingsClicked()
{
	UE_LOG(LogSeagullStorm, Log, TEXT("Settings panel requested"));
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

	UE_LOG(LogSeagullStorm, Log, TEXT("Bought %s upgrade: level %d -> %d (cost %d)"), *Key, CurrentLevel, CurrentLevel + 1, Cost);

	RefreshDisplay();
}
