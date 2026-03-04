#include "UI/SeagullFeedbackForm.h"
#include "Core/SeagullGameInstance.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "SeagullStorm.h"

void USeagullFeedbackForm::NativeConstruct()
{
	Super::NativeConstruct();

	if (SubmitButton) SubmitButton->OnClicked.AddDynamic(this, &USeagullFeedbackForm::OnSubmitClicked);
	if (CloseButton) CloseButton->OnClicked.AddDynamic(this, &USeagullFeedbackForm::OnCloseClicked);

	if (CategoryDropdown)
	{
		CategoryDropdown->AddOption(TEXT("BUG"));
		CategoryDropdown->AddOption(TEXT("FEATURE"));
		CategoryDropdown->AddOption(TEXT("GENERAL"));
		CategoryDropdown->SetSelectedOption(TEXT("GENERAL"));
	}
}

void USeagullFeedbackForm::OnSubmitClicked()
{
	FString Title = TitleInput ? TitleInput->GetText().ToString() : TEXT("");
	FString Message = MessageInput ? MessageInput->GetText().ToString() : TEXT("");
	FString Category = CategoryDropdown ? CategoryDropdown->GetSelectedOption() : TEXT("GENERAL");

	if (Title.IsEmpty() || Message.IsEmpty())
	{
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("Please fill in both fields")));
		return;
	}

	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	USeagullHorizonManager* HM = GI->GetHorizonManager();
	if (!HM) return;

	if (StatusText) StatusText->SetText(FText::FromString(TEXT("Submitting...")));

	HM->SubmitFeedback(Title, Message, Category, [this](bool bSuccess)
	{
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(bSuccess ? TEXT("Thank you!") : TEXT("Failed to submit")));
		}
	});
}

void USeagullFeedbackForm::OnCloseClicked()
{
	RemoveFromParent();
}
