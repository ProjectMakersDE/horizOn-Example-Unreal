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

	// Read from cache — zero network requests during run
	for (const FHorizonNewsEntry& Entry : GI->CachedNews)
	{
		UE_LOG(LogSeagullStorm, Log, TEXT("News: %s (%s)"), *Entry.Title, *Entry.ReleaseDate);
	}
}

void USeagullNewsPanel::OnCloseClicked()
{
	RemoveFromParent();
}
