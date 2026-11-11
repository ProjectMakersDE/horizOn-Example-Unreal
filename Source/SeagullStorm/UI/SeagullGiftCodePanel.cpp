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
	RedeemButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Prepare"),
		SeagullColors::Orange, SeagullColors::DarkText, SeagullWidgetStyles::TextFontSize, TEXT("RedeemButton"));
	if (UHorizontalBoxSlot* RedeemSlot = ButtonRow->AddChildToHorizontalBox(RedeemButton))
	{
		RedeemSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RedeemSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
	}
    ConfirmButton=SeagullWidgetStyles::MakeTextButton(WidgetTree,TEXT("Confirm / retry saved"),SeagullColors::Teal,SeagullColors::LightText,SeagullWidgetStyles::SmallFontSize,TEXT("ConfirmGiftButton"));
    if(auto* ConfirmSlot=ButtonRow->AddChildToHorizontalBox(ConfirmButton)){ConfirmSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));ConfirmSlot->SetPadding(FMargin(4.f,0.f));}
	CloseButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Close"),
		SeagullColors::Teal, SeagullColors::LightText, SeagullWidgetStyles::TextFontSize, TEXT("CloseButton"));
	if (UHorizontalBoxSlot* CloseSlot = ButtonRow->AddChildToHorizontalBox(CloseButton))
	{
		CloseSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		CloseSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));
	}
	SeagullWidgetStyles::AddRow(Column, ButtonRow, 10.f);
    SetMenuFirstFocus(CodeInput);
    SetMenuBack(FSimpleDelegate::CreateUObject(this,&USeagullGiftCodePanel::OnCloseClicked));
}

void USeagullGiftCodePanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (RedeemButton) RedeemButton->OnClicked.AddUniqueDynamic(this, &USeagullGiftCodePanel::OnRedeemClicked);
	if (CloseButton) CloseButton->OnClicked.AddUniqueDynamic(this, &USeagullGiftCodePanel::OnCloseClicked);
    if(ConfirmButton)ConfirmButton->OnClicked.AddUniqueDynamic(this,&USeagullGiftCodePanel::OnConfirmClicked);
    CompleteRequest(TEXT("Prepare reads a V2 intent. Confirm or retry always sends the saved original tuple. Server balances are separate from practice coins."));
}

void USeagullGiftCodePanel::NativeTick(const FGeometry& Geometry,float DeltaTime)
{
    Super::NativeTick(Geometry,DeltaTime);
    auto* GI=Cast<USeagullGameInstance>(GetGameInstance());auto* HM=GI?GI->GetHorizonManager():nullptr;
    // Local UI state only. A recreated panel can observe the original request's
    // completion without issuing a hidden read or inventing another tuple.
    if(HM&&HM->IsGiftInFlight()!=bObservedBusy&&!bRedeeming)CompleteRequest(TEXT("Saved original state refreshed. Confirm/retry the same tuple or prepare a new gift when none remains."));
}
void USeagullGiftCodePanel::OnRedeemClicked()
{
    auto* GI=Cast<USeagullGameInstance>(GetGameInstance());auto* HM=GI?GI->GetHorizonManager():nullptr;
    if(bRedeeming||!HM||!HM->IsSignedIn()||GI->bPracticeMode){CompleteRequest(TEXT("Sign in to prepare an online gift."));return;}
    const FString Code=CodeInput?CodeInput->GetText().ToString().TrimStartAndEnd():FString();
    if(Code.IsEmpty()){CompleteRequest(TEXT("Enter a code."));return;}
    bRedeeming=true;RedeemButton->SetIsEnabled(false);ConfirmButton->SetIsEnabled(false);
    HM->PrepareGiftCode(Code,[Weak=TWeakObjectPtr<USeagullGiftCodePanel>(this)](bool Ok,const FHorizonGiftCodeIntent& Intent,const FString& Error)
    {if(auto* Panel=Weak.Get())Panel->CompleteRequest(Ok?FString::Printf(TEXT("Prepared %s. Original count %s, revision %s. Confirm the saved tuple."),*Intent.CampaignId,*Intent.ExpectedCount,*Intent.ConfigRevision):TEXT("Preparation stopped: ")+Error);});
}
void USeagullGiftCodePanel::OnConfirmClicked()
{
    auto* GI=Cast<USeagullGameInstance>(GetGameInstance());auto* HM=GI?GI->GetHorizonManager():nullptr;
    if(bRedeeming||!HM||!HM->IsSignedIn()||GI->bPracticeMode){CompleteRequest(TEXT("Sign in to confirm the saved original."));return;}
    bRedeeming=true;RedeemButton->SetIsEnabled(false);ConfirmButton->SetIsEnabled(false);
    HM->RedeemPendingGift([Weak=TWeakObjectPtr<USeagullGiftCodePanel>(this)](bool Ok,const FHorizonGiftCodeRedeemResult& Result,const FString& Error)
    {if(auto* Panel=Weak.Get())Panel->CompleteRequest(Ok?FString::Printf(TEXT("Server accepted original campaign %s at exact count %s (%s). Read current server balances in SDK tools. Practice coins are unchanged."),*Result.CampaignId,*Result.Count,Result.bAlreadyApplied?TEXT("already applied"):TEXT("fresh")):TEXT("Original retained. Explicit retry uses the same tuple: ")+Error);});
}
void USeagullGiftCodePanel::CompleteRequest(const FString& Message)
{
    bRedeeming=false;
    auto* GI=Cast<USeagullGameInstance>(GetGameInstance());auto* HM=GI?GI->GetHorizonManager():nullptr;
    FHorizonGiftCodeIntent Intent;FString Error;const bool Valid=HM&&HM->ReadPendingGift(Intent,Error),Pending=Valid&&!Intent.CampaignId.IsEmpty(),Busy=HM&&HM->IsGiftInFlight();
    bObservedBusy=Busy;
    if(RedeemButton)RedeemButton->SetIsEnabled(Valid&&!Pending&&!Busy);
    if(ConfirmButton)ConfirmButton->SetIsEnabled(Valid&&Pending&&!Busy);
    if(StatusText)StatusText->SetText(FText::FromString(Valid?Message:TEXT("Stored original/session unavailable: ")+Error));
}

void USeagullGiftCodePanel::OnCloseClicked()
{
	RemoveFromParent();
}
