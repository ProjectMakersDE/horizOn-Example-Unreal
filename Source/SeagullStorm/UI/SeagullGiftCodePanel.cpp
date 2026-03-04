#include "UI/SeagullGiftCodePanel.h"
#include "Core/SeagullGameInstance.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "SeagullStorm.h"

void USeagullGiftCodePanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (RedeemButton) RedeemButton->OnClicked.AddDynamic(this, &USeagullGiftCodePanel::OnRedeemClicked);
	if (CloseButton) CloseButton->OnClicked.AddDynamic(this, &USeagullGiftCodePanel::OnCloseClicked);
}

void USeagullGiftCodePanel::OnRedeemClicked()
{
	FString Code = CodeInput ? CodeInput->GetText().ToString() : TEXT("");
	if (Code.IsEmpty())
	{
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("Enter a code")));
		return;
	}

	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	// Check if already redeemed
	if (GI->SaveData.GiftCodesRedeemed.Contains(Code))
	{
		if (StatusText) StatusText->SetText(FText::FromString(TEXT("Already redeemed")));
		return;
	}

	USeagullHorizonManager* HM = GI->GetHorizonManager();
	if (!HM) return;

	if (StatusText) StatusText->SetText(FText::FromString(TEXT("Validating...")));

	// Validate first, then redeem
	HM->ValidateGiftCode(Code, [this, Code, GI, HM](bool bRequestSuccess, bool bValid)
	{
		if (!bRequestSuccess || !bValid)
		{
			if (StatusText) StatusText->SetText(FText::FromString(TEXT("Invalid code")));
			return;
		}

		HM->RedeemGiftCode(Code, [this, Code, GI](bool bSuccess, const FString& GiftData, const FString& Message)
		{
			if (bSuccess)
			{
				GI->SaveData.GiftCodesRedeemed.Add(Code);
				// Parse gift data for coins (simplified)
				GI->SaveData.Coins += 500; // Default reward
				if (StatusText) StatusText->SetText(FText::FromString(TEXT("Code redeemed! +500 coins")));
				UE_LOG(LogSeagullStorm, Log, TEXT("Gift code redeemed: %s"), *Code);
			}
			else
			{
				if (StatusText) StatusText->SetText(FText::FromString(TEXT("Redemption failed")));
			}
		});
	});
}

void USeagullGiftCodePanel::OnCloseClicked()
{
	RemoveFromParent();
}
