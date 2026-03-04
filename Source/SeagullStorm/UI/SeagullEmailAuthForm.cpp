#include "UI/SeagullEmailAuthForm.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameMode.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "SeagullStorm.h"

void USeagullEmailAuthForm::NativeConstruct()
{
	Super::NativeConstruct();

	if (SubmitButton) SubmitButton->OnClicked.AddDynamic(this, &USeagullEmailAuthForm::OnSubmitClicked);
	if (CloseButton) CloseButton->OnClicked.AddDynamic(this, &USeagullEmailAuthForm::OnCloseClicked);

	if (TitleLabel)
	{
		TitleLabel->SetText(FText::FromString(bRegisterMode ? TEXT("Create Account") : TEXT("Sign In")));
	}

	// Hide username field for sign-in mode
	if (UsernameInput && !bRegisterMode)
	{
		UsernameInput->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USeagullEmailAuthForm::OnSubmitClicked()
{
	FString Email = EmailInput ? EmailInput->GetText().ToString() : TEXT("");
	FString Password = PasswordInput ? PasswordInput->GetText().ToString() : TEXT("");

	if (Email.IsEmpty() || Password.IsEmpty())
	{
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("Fill in all fields")));
		return;
	}

	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	USeagullHorizonManager* HM = GI->GetHorizonManager();
	if (!HM) return;

	if (StatusText) StatusText->SetText(FText::FromString(TEXT("Authenticating...")));

	if (bRegisterMode)
	{
		FString Username = UsernameInput ? UsernameInput->GetText().ToString() : TEXT("Player");
		HM->SignUpEmail(Email, Password, Username, [this](bool bSuccess) { OnAuthResult(bSuccess); });
	}
	else
	{
		HM->SignInEmail(Email, Password, [this](bool bSuccess) { OnAuthResult(bSuccess); });
	}
}

void USeagullEmailAuthForm::OnCloseClicked()
{
	RemoveFromParent();
}

void USeagullEmailAuthForm::OnAuthResult(bool bSuccess)
{
	if (bSuccess)
	{
		RemoveFromParent();
		ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GM)
		{
			GM->SwitchToScreen(ESeagullGameScreen::Hub);
		}
	}
	else
	{
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("Authentication failed")));
	}
}
