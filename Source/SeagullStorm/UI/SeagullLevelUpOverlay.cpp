#include "UI/SeagullLevelUpOverlay.h"
#include "Core/SeagullStormGameState.h"
#include "Core/SeagullPlayerController.h"
#include "Core/SeagullGameInstance.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "SeagullStorm.h"

void USeagullLevelUpOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	if (Choice1Button) Choice1Button->OnClicked.AddDynamic(this, &USeagullLevelUpOverlay::OnChoice1);
	if (Choice2Button) Choice2Button->OnClicked.AddDynamic(this, &USeagullLevelUpOverlay::OnChoice2);
	if (Choice3Button) Choice3Button->OnClicked.AddDynamic(this, &USeagullLevelUpOverlay::OnChoice3);
}

void USeagullLevelUpOverlay::SetChoices(const TArray<FSeagullLevelUpChoice>& InChoices)
{
	Choices = InChoices;

	UTextBlock* Texts[] = { Choice1Text, Choice2Text, Choice3Text };
	for (int32 i = 0; i < 3; i++)
	{
		if (Texts[i] && Choices.IsValidIndex(i))
		{
			Texts[i]->SetText(FText::FromString(
				FString::Printf(TEXT("%s\n%s"), *Choices[i].DisplayName, *Choices[i].Description)));
		}
	}
}

void USeagullLevelUpOverlay::OnChoice1() { SelectChoice(0); }
void USeagullLevelUpOverlay::OnChoice2() { SelectChoice(1); }
void USeagullLevelUpOverlay::OnChoice3() { SelectChoice(2); }

void USeagullLevelUpOverlay::SelectChoice(int32 Index)
{
	if (!Choices.IsValidIndex(Index)) return;

	const FSeagullLevelUpChoice& Choice = Choices[Index];

	// Apply to game state
	ASeagullStormGameState* GS = GetWorld()->GetGameState<ASeagullStormGameState>();
	if (GS)
	{
		GS->ApplyLevelUpChoice(Choice);
	}

	// Record breadcrumb
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (GI)
	{
		USeagullHorizonManager* HM = GI->GetHorizonManager();
		if (HM)
		{
			HM->RecordBreadcrumb(TEXT("user_action"), FString::Printf(TEXT("chose_%s"), *Choice.Id));
			if (GS) HM->SetCrashCustomKey(TEXT("level"), FString::FromInt(GS->CurrentLevel));
		}
	}

	UE_LOG(LogSeagullStorm, Log, TEXT("Level-up choice: %s"), *Choice.Id);

	// Hide overlay and resume
	ASeagullPlayerController* PC = Cast<ASeagullPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC) PC->HideLevelUpOverlay();
}
