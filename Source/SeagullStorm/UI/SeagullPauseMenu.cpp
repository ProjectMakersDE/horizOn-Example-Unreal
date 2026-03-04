#include "UI/SeagullPauseMenu.h"
#include "Core/SeagullPlayerController.h"
#include "Core/SeagullStormGameMode.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "SeagullStorm.h"

void USeagullPauseMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton) ResumeButton->OnClicked.AddDynamic(this, &USeagullPauseMenu::OnResumeClicked);
	if (NewsButton) NewsButton->OnClicked.AddDynamic(this, &USeagullPauseMenu::OnNewsClicked);
	if (FeedbackButton) FeedbackButton->OnClicked.AddDynamic(this, &USeagullPauseMenu::OnFeedbackClicked);
	if (QuitButton) QuitButton->OnClicked.AddDynamic(this, &USeagullPauseMenu::OnQuitClicked);
}

void USeagullPauseMenu::OnResumeClicked()
{
	ASeagullPlayerController* PC = Cast<ASeagullPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC) PC->TogglePause();
}

void USeagullPauseMenu::OnNewsClicked()
{
	UE_LOG(LogSeagullStorm, Log, TEXT("News panel requested from pause menu"));
}

void USeagullPauseMenu::OnFeedbackClicked()
{
	UE_LOG(LogSeagullStorm, Log, TEXT("Feedback form requested from pause menu"));
}

void USeagullPauseMenu::OnQuitClicked()
{
	// Quit counts as death for scoring
	ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		ASeagullPlayerController* PC = Cast<ASeagullPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (PC)
		{
			PC->SetPause(false);
		}
		GM->EndRun(true);
	}
}
