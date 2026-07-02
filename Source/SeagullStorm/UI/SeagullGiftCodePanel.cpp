#include "UI/SeagullGiftCodePanel.h"
#include "UI/SeagullWidgetStyles.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullPlayerController.h"
#include "Core/SeagullTypes.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "SeagullStorm.h"

void USeagullGiftCodePanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	FLinearColor DimColor = SeagullColors::DarkBG;
	DimColor.A = 0.85f;
	UBorder* Dim = SeagullWidgetStyles::MakePanel(WidgetTree, DimColor, 0.f);
	if (UCanvasPanelSlot* DimSlot = RootCanvas->AddChildToCanvas(Dim))
	{
		DimSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		DimSlot->SetOffsets(FMargin(0.f));
	}

	UBorder* Panel = SeagullWidgetStyles::MakePanel(WidgetTree, SeagullColors::PanelBG, 32.f);
	if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Panel))
	{
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetAutoSize(true);
	}

	USizeBox* ColumnSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ColumnSize->SetWidthOverride(620.f);
	Panel->AddChild(ColumnSize);

	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	ColumnSize->AddChild(Column);

	UTextBlock* Heading = SeagullWidgetStyles::MakeText(WidgetTree, TEXT("GIFT CODE"),
		SeagullWidgetStyles::TextFontSize, SeagullColors::Orange);
	Heading->SetJustification(ETextJustify::Center);
	SeagullWidgetStyles::AddRow(Column, Heading, 10.f);

	CodeInput = SeagullWidgetStyles::MakeTextInput(WidgetTree, TEXT("Enter your code..."), TEXT("CodeInput"));
	SeagullWidgetStyles::AddRow(Column, CodeInput);

	StatusText = SeagullWidgetStyles::MakeText(WidgetTree, TEXT(""),
		SeagullWidgetStyles::SmallFontSize, SeagullColors::Orange, TEXT("StatusText"));
	StatusText->SetJustification(ETextJustify::Center);
	StatusText->SetAutoWrapText(true);
	SeagullWidgetStyles::AddRow(Column, StatusText, 6.f);

	UHorizontalBox* ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	RedeemButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Redeem"),
		SeagullColors::Orange, SeagullColors::DarkText, SeagullWidgetStyles::TextFontSize, TEXT("RedeemButton"));
	if (UHorizontalBoxSlot* RedeemSlot = ButtonRow->AddChildToHorizontalBox(RedeemButton))
	{
		RedeemSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RedeemSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
	}
	CloseButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Close"),
		SeagullColors::Teal, SeagullColors::LightText, SeagullWidgetStyles::TextFontSize, TEXT("CloseButton"));
	if (UHorizontalBoxSlot* CloseSlot = ButtonRow->AddChildToHorizontalBox(CloseButton))
	{
		CloseSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		CloseSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));
	}
	SeagullWidgetStyles::AddRow(Column, ButtonRow, 10.f);
}

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

	// Validate first, then redeem. Weak self: the panel can be closed and GC'd
	// while the requests are in flight; GI and HM live on the GameInstance.
	TWeakObjectPtr<USeagullGiftCodePanel> WeakThis(this);
	HM->ValidateGiftCode(Code, [WeakThis, Code, GI, HM](bool bRequestSuccess, bool bValid)
	{
		if (!bRequestSuccess || !bValid)
		{
			USeagullGiftCodePanel* Panel = WeakThis.Get();
			if (Panel && Panel->StatusText) Panel->StatusText->SetText(FText::FromString(TEXT("Invalid code")));
			return;
		}

		HM->RedeemGiftCode(Code, [WeakThis, Code, GI, HM](bool bSuccess, const FString& GiftData, const FString& Message)
		{
			USeagullGiftCodePanel* Panel = WeakThis.Get();
			if (bSuccess)
			{
				GI->SaveData.GiftCodesRedeemed.Add(Code);
				// Parse gift data for coin reward
				int32 CoinReward = 0;
				TSharedPtr<FJsonObject> JsonObj;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(GiftData);
				if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
				{
					CoinReward = JsonObj->GetIntegerField(TEXT("coins"));
				}
				if (CoinReward <= 0) CoinReward = 500; // Fallback
				GI->SaveData.Coins += CoinReward;

				// Persist right away -- the server already consumed the single-use
				// code, so the reward must survive quitting before the next
				// game-over save (on-demand request, within the design budget).
				HM->SaveData(GI->SaveData.ToJsonString(), [HM](bool bSaveSuccess)
				{
					if (!bSaveSuccess && HM)
					{
						HM->RecordException(
							TEXT("SaveData failed"),
							TEXT("CloudSave SaveData returned bSuccess=false after gift code redemption"));
					}
				});

				if (Panel)
				{
					if (Panel->StatusText) Panel->StatusText->SetText(FText::FromString(
						FString::Printf(TEXT("Code redeemed! +%d coins"), CoinReward)));

					// Update the hub's coin counter behind the modal
					ASeagullPlayerController* PC = Cast<ASeagullPlayerController>(
						UGameplayStatics::GetPlayerController(Panel->GetWorld(), 0));
					if (PC) PC->RefreshHubDisplay();
				}
				UE_LOG(LogSeagullStorm, Log, TEXT("Gift code redeemed: %s (+%d coins)"), *Code, CoinReward);
			}
			else
			{
				if (Panel && Panel->StatusText) Panel->StatusText->SetText(FText::FromString(TEXT("Redemption failed")));
			}
		});
	});
}

void USeagullGiftCodePanel::OnCloseClicked()
{
	RemoveFromParent();
}
