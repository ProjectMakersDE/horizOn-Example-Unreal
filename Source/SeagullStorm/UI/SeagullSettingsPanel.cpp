#include "UI/SeagullSettingsPanel.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameMode.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "SeagullStorm.h"

void USeagullSettingsPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (SignOutButton) SignOutButton->OnClicked.AddDynamic(this, &USeagullSettingsPanel::OnSignOutClicked);
	if (CloseButton) CloseButton->OnClicked.AddDynamic(this, &USeagullSettingsPanel::OnCloseClicked);

	// Display user info
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (GI && UserInfoText)
	{
		USeagullHorizonManager* HM = GI->GetHorizonManager();
		if (HM)
		{
			UserInfoText->SetText(FText::FromString(
				FString::Printf(TEXT("Signed in as: %s"), *HM->GetDisplayName())));
		}
	}
}

void USeagullSettingsPanel::OnSignOutClicked()
{
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (GI)
	{
		USeagullHorizonManager* HM = GI->GetHorizonManager();
		if (HM)
		{
			HM->SignOut();
			GI->bSaveLoaded = false;
			GI->bConfigLoaded = false;
			GI->SaveData.InitDefaults();

			ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			if (GM)
			{
				GM->SwitchToScreen(ESeagullGameScreen::Title);
			}
		}
	}
}

void USeagullSettingsPanel::OnCloseClicked()
{
	RemoveFromParent();
}
