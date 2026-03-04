#include "UI/SeagullHUD.h"
#include "Core/SeagullStormGameState.h"
#include "Core/SeagullPlayerController.h"
#include "Player/SeagullPlayerPawn.h"
#include "Player/SeagullHealthComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void USeagullHUD::NativeConstruct()
{
	Super::NativeConstruct();
	if (PauseButton) PauseButton->OnClicked.AddDynamic(this, &USeagullHUD::OnPauseClicked);
}

void USeagullHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ASeagullStormGameState* GS = GetWorld()->GetGameState<ASeagullStormGameState>();
	if (!GS) return;

	if (WaveText)
		WaveText->SetText(FText::FromString(FString::Printf(TEXT("Wave %d"), GS->CurrentWave)));

	if (TimerText)
	{
		int32 Seconds = FMath::Max(0, static_cast<int32>(GS->TimeRemaining));
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), Seconds / 60, Seconds % 60)));
	}

	if (ScoreText)
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), GS->CurrentScore)));

	if (LevelText)
		LevelText->SetText(FText::FromString(FString::Printf(TEXT("Lv. %d"), GS->CurrentLevel)));

	// HP bar
	ASeagullPlayerPawn* Player = Cast<ASeagullPlayerPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (Player && Player->HealthComponent && HPBar)
	{
		float Pct = static_cast<float>(Player->HealthComponent->CurrentHP) /
			FMath::Max(1, Player->HealthComponent->MaxHP);
		HPBar->SetPercent(Pct);
	}

	// XP bar
	if (XPBar && GS->XPToNextLevel > 0)
	{
		float Pct = static_cast<float>(GS->CurrentXP) / static_cast<float>(GS->XPToNextLevel);
		XPBar->SetPercent(Pct);
	}
}

void USeagullHUD::OnPauseClicked()
{
	ASeagullPlayerController* PC = Cast<ASeagullPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC) PC->TogglePause();
}
