#include "UI/SeagullPauseMenu.h"
#include "UI/SeagullNewsPanel.h"
#include "UI/SeagullFeedbackForm.h"
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
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	USeagullNewsPanel* Panel = CreateWidget<USeagullNewsPanel>(PC);
	if (Panel) Panel->AddToViewport(200);
}

void USeagullPauseMenu::OnFeedbackClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	USeagullFeedbackForm* Form = CreateWidget<USeagullFeedbackForm>(PC);
	if (Form) Form->AddToViewport(200);
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
