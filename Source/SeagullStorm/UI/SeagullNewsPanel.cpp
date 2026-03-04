#include "UI/SeagullNewsPanel.h"
#include "Core/SeagullGameInstance.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "SeagullStorm.h"

void USeagullNewsPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton) CloseButton->OnClicked.AddDynamic(this, &USeagullNewsPanel::OnCloseClicked);
	LoadNews();
}

void USeagullNewsPanel::LoadNews()
{
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	// Populate NewsList widget from cache — zero network requests during run
	if (NewsList)
	{
		NewsList->ClearChildren();
		for (const FHorizonNewsEntry& Entry : GI->CachedNews)
		{
			UTextBlock* Row = NewObject<UTextBlock>(this);
			Row->SetText(FText::FromString(
				FString::Printf(TEXT("[%s] %s"), *Entry.ReleaseDate, *Entry.Title)));
			NewsList->AddChildToVerticalBox(Row);
		}
	}
	UE_LOG(LogSeagullStorm, Log, TEXT("News panel loaded: %d entries"), GI->CachedNews.Num());
}

void USeagullNewsPanel::OnCloseClicked()
{
	RemoveFromParent();
}
