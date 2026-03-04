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

	USeagullHorizonManager* HM = GI->GetHorizonManager();
	if (!HM) return;

	HM->LoadNews(5, TEXT("en"), [this](bool bSuccess, const TArray<FHorizonNewsEntry>& Entries)
	{
		if (bSuccess)
		{
			for (const FHorizonNewsEntry& Entry : Entries)
			{
				UE_LOG(LogSeagullStorm, Log, TEXT("News: %s (%s)"), *Entry.Title, *Entry.ReleaseDate);
			}
		}
	});
}

void USeagullNewsPanel::OnCloseClicked()
{
	RemoveFromParent();
}
