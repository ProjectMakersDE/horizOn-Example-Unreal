#include "UI/SeagullTitleScreen.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameMode.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Kismet/GameplayStatics.h"
#include "SeagullStorm.h"

void USeagullTitleScreen::NativeConstruct()
{
	Super::NativeConstruct();

	// Build UI programmatically since we don't use Blueprint widgets
	// In a real UE project, these would be constructed via WidgetTree
	// For this SDK example, the UI structure demonstrates the flow

	if (GuestButton) GuestButton->OnClicked.AddDynamic(this, &USeagullTitleScreen::OnGuestClicked);
	if (GoogleButton) GoogleButton->OnClicked.AddDynamic(this, &USeagullTitleScreen::OnGoogleClicked);
	if (EmailButton) EmailButton->OnClicked.AddDynamic(this, &USeagullTitleScreen::OnEmailClicked);
	if (RegisterButton) RegisterButton->OnClicked.AddDynamic(this, &USeagullTitleScreen::OnRegisterClicked);
}

void USeagullTitleScreen::OnGuestClicked()
{
	FString Name = NameInput ? NameInput->GetText().ToString() : TEXT("Player");
	if (Name.IsEmpty()) Name = TEXT("Player");

	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	USeagullHorizonManager* HM = GI->GetHorizonManager();
	if (!HM) return;

	HM->SignUpAnonymous(Name, [this](bool bSuccess)
	{
		if (bSuccess) OnAuthSuccess();
		else OnAuthFailure();
	});
}

void USeagullTitleScreen::OnGoogleClicked()
{
	// Google OAuth not implemented in this example
	UE_LOG(LogSeagullStorm, Log, TEXT("Google sign-in not available in this example"));
}

void USeagullTitleScreen::OnEmailClicked()
{
	// Would show email auth form overlay
	UE_LOG(LogSeagullStorm, Log, TEXT("Email sign-in clicked -- would show email form"));
}

void USeagullTitleScreen::OnRegisterClicked()
{
	// Would show registration form overlay
	UE_LOG(LogSeagullStorm, Log, TEXT("Register clicked -- would show registration form"));
}

void USeagullTitleScreen::OnAuthSuccess()
{
	UE_LOG(LogSeagullStorm, Log, TEXT("Authentication successful"));

	ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		GM->SwitchToScreen(ESeagullGameScreen::Hub);
	}
}

void USeagullTitleScreen::OnAuthFailure()
{
	UE_LOG(LogSeagullStorm, Warning, TEXT("Authentication failed"));
}
