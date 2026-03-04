#include "UI/SeagullTitleScreen.h"
#include "UI/SeagullEmailAuthForm.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameMode.h"
#include "Horizon/SeagullHorizonManager.h"
#include "HorizonSubsystem.h"
#include "Managers/HorizonAuthManager.h"
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
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	UHorizonSubsystem* Subsystem = GI->GetHorizonSubsystem();
	if (Subsystem && Subsystem->Auth)
	{
		// Attempt SDK Google sign-in. On platforms without a real Google auth code
		// this will fail, which is handled in the callback.
		Subsystem->Auth->SignInGoogle(TEXT(""), TEXT(""), FOnAuthComplete::CreateLambda(
			[this](bool bSuccess)
			{
				if (bSuccess)
				{
					OnAuthSuccess();
				}
				else
				{
					if (StatusText)
					{
						StatusText->SetText(FText::FromString(TEXT("Google sign-in is not available on this platform")));
					}
					else
					{
						UE_LOG(LogSeagullStorm, Warning, TEXT("Google sign-in not available on this platform"));
					}
				}
			}));
	}
	else
	{
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(TEXT("Google sign-in is not available on this platform")));
		}
		else
		{
			UE_LOG(LogSeagullStorm, Warning, TEXT("Google sign-in not available on this platform"));
		}
	}
}

void USeagullTitleScreen::OnEmailClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	USeagullEmailAuthForm* Form = CreateWidget<USeagullEmailAuthForm>(PC);
	if (Form)
	{
		Form->bRegisterMode = false;
		Form->AddToViewport(100);
	}
}

void USeagullTitleScreen::OnRegisterClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	USeagullEmailAuthForm* Form = CreateWidget<USeagullEmailAuthForm>(PC);
	if (Form)
	{
		Form->bRegisterMode = true;
		Form->AddToViewport(100);
	}
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
