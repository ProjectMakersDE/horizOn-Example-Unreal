#include "UI/SeagullGameOverScreen.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameMode.h"
#include "Core/SeagullStormGameState.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "SeagullStorm.h"

void USeagullGameOverScreen::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayAgainButton) PlayAgainButton->OnClicked.AddDynamic(this, &USeagullGameOverScreen::OnPlayAgainClicked);
	if (HubButton) HubButton->OnClicked.AddDynamic(this, &USeagullGameOverScreen::OnHubClicked);

	LoadGameOverData();
}

void USeagullGameOverScreen::LoadGameOverData()
{
	ASeagullStormGameState* GS = GetWorld()->GetGameState<ASeagullStormGameState>();
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	if (GS)
	{
		if (ScoreText) ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), GS->CurrentScore)));
		if (WavesText) WavesText->SetText(FText::FromString(FString::Printf(TEXT("Waves: %d"), GS->CurrentWave)));
		if (LevelText) LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), GS->CurrentLevel)));
		if (CoinsText) CoinsText->SetText(FText::FromString(FString::Printf(TEXT("Coins: +%d"), GS->RunStats.CoinsEarned)));
	}

	if (GI)
	{
		if (BestText) BestText->SetText(FText::FromString(FString::Printf(TEXT("Best: %d"), GI->SaveData.Highscore)));
	}

	// Get rank from leaderboard
	if (GI)
	{
		USeagullHorizonManager* HM = GI->GetHorizonManager();
		if (HM)
		{
			HM->GetRank([this](bool bSuccess, const FHorizonLeaderboardEntry& Entry)
			{
				if (bSuccess && RankText)
				{
					RankText->SetText(FText::FromString(FString::Printf(TEXT("Rank: #%d"), Entry.Position)));
				}
			});
		}
	}
}

void USeagullGameOverScreen::OnPlayAgainClicked()
{
	ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM) GM->StartRun();
}

void USeagullGameOverScreen::OnHubClicked()
{
	ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM) GM->ReturnToHub();
}
