#include "UI/SeagullTitleScreen.h"
#include "UI/SeagullEmailAuthForm.h"
#include "UI/SeagullWidgetStyles.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameMode.h"
#include "Core/SeagullTypes.h"
#include "Horizon/SeagullHorizonManager.h"
#include "HorizonSubsystem.h"
#include "Managers/HorizonAuthManager.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "SeagullStorm.h"

void USeagullTitleScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Build the whole widget tree in C++; this project uses no Blueprint widgets.
	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	// Full-screen dark background
	UBorder* Background = SeagullWidgetStyles::MakePanel(WidgetTree, SeagullColors::DarkBG, 0.f);
	if (UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(Background))
	{
		BackgroundSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		BackgroundSlot->SetOffsets(FMargin(0.f));
	}

	// Centered column
	USizeBox* ColumnSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ColumnSize->SetWidthOverride(680.f);
	if (UCanvasPanelSlot* ColumnSlot = RootCanvas->AddChildToCanvas(ColumnSize))
	{
		ColumnSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		ColumnSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		ColumnSlot->SetAutoSize(true);
	}

	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	ColumnSize->AddChild(Column);

	// Seagull logo, imported from the shipped PNG at runtime, so no .uasset is needed
	USizeBox* LogoSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	LogoSize->SetWidthOverride(192.f);
	LogoSize->SetHeightOverride(192.f);
	LogoImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("LogoImage"));
	UTexture2D* LogoTexture = FImageUtils::ImportFileAsTexture2D(
		FPaths::ProjectContentDir() / TEXT("Textures/seagull_logo.png"));
	if (LogoTexture)
	{
		// The sampler state was baked when ImportFileAsTexture2D initialized the
		// resource; re-init so TF_Nearest actually applies (crisp pixel art).
		LogoTexture->Filter = TextureFilter::TF_Nearest;
		LogoTexture->UpdateResource();
		LogoImage->SetBrushFromTexture(LogoTexture, false);
	}
	else
	{
		LogoSize->SetVisibility(ESlateVisibility::Collapsed);
	}
	LogoSize->AddChild(LogoImage);
	if (UVerticalBoxSlot* LogoSlot = Column->AddChildToVerticalBox(LogoSize))
	{
		LogoSlot->SetHorizontalAlignment(HAlign_Center);
		LogoSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));
	}

	// Title + subtitle
	TitleText = SeagullWidgetStyles::MakeText(WidgetTree, TEXT("SEAGULL STORM"),
		SeagullWidgetStyles::TitleFontSize, SeagullColors::Orange, TEXT("TitleText"));
	TitleText->SetJustification(ETextJustify::Center);
	SeagullWidgetStyles::AddRow(Column, TitleText, 6.f);

	SubtitleText = SeagullWidgetStyles::MakeText(WidgetTree, TEXT("horizOn SDK Example"),
		SeagullWidgetStyles::SmallFontSize, SeagullColors::LightText, TEXT("SubtitleText"));
	SubtitleText->SetJustification(ETextJustify::Center);
	SeagullWidgetStyles::AddRow(Column, SubtitleText, 6.f);

	// Name input
	NameInput = SeagullWidgetStyles::MakeTextInput(WidgetTree, TEXT("Enter your name..."), TEXT("NameInput"));
	SeagullWidgetStyles::AddRow(Column, NameInput, 14.f);

	// Auth buttons
	GuestButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Play as Guest"),
		SeagullColors::Orange, SeagullColors::DarkText, SeagullWidgetStyles::TextFontSize, TEXT("GuestButton"));
	SeagullWidgetStyles::AddRow(Column, GuestButton);

	GoogleButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Sign In with Google"),
		SeagullColors::Teal, SeagullColors::LightText, SeagullWidgetStyles::TextFontSize, TEXT("GoogleButton"));
	SeagullWidgetStyles::AddRow(Column, GoogleButton);

	AppleButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Sign In with Apple"),
		SeagullColors::Teal, SeagullColors::LightText, SeagullWidgetStyles::TextFontSize, TEXT("AppleButton"));
	SeagullWidgetStyles::AddRow(Column, AppleButton);

	EmailButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Sign In with Email"),
		SeagullColors::Teal, SeagullColors::LightText, SeagullWidgetStyles::TextFontSize, TEXT("EmailButton"));
	SeagullWidgetStyles::AddRow(Column, EmailButton);

	RegisterButton = SeagullWidgetStyles::MakeTextButton(WidgetTree, TEXT("Create Account"),
		SeagullColors::Teal, SeagullColors::LightText, SeagullWidgetStyles::TextFontSize, TEXT("RegisterButton"));
	SeagullWidgetStyles::AddRow(Column, RegisterButton);

	// Status line (auth errors etc.)
	StatusText = SeagullWidgetStyles::MakeText(WidgetTree, TEXT(""),
		SeagullWidgetStyles::SmallFontSize, SeagullColors::Orange, TEXT("StatusText"));
	StatusText->SetJustification(ETextJustify::Center);
	StatusText->SetAutoWrapText(true);
	SeagullWidgetStyles::AddRow(Column, StatusText, 12.f);

	// Footer
	UTextBlock* Footer = SeagullWidgetStyles::MakeText(WidgetTree, TEXT("horizon.pm"),
		SeagullWidgetStyles::SmallFontSize, FLinearColor::FromSRGBColor(FColor(0x66, 0x66, 0x66)));
	Footer->SetJustification(ETextJustify::Center);
	SeagullWidgetStyles::AddRow(Column, Footer, 4.f);
}

void USeagullTitleScreen::NativeConstruct()
{
	Super::NativeConstruct();

	if (GuestButton) GuestButton->OnClicked.AddDynamic(this, &USeagullTitleScreen::OnGuestClicked);
	if (GoogleButton) GoogleButton->OnClicked.AddDynamic(this, &USeagullTitleScreen::OnGoogleClicked);
	if (AppleButton) AppleButton->OnClicked.AddDynamic(this, &USeagullTitleScreen::OnAppleClicked);
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

	// Weak self: the title screen can be removed and GC'd (e.g. a concurrent
	// session restore switching to the Hub) before the HTTP response lands.
	TWeakObjectPtr<USeagullTitleScreen> WeakThis(this);
	HM->SignUpAnonymous(Name, [WeakThis](bool bSuccess)
	{
		USeagullTitleScreen* Screen = WeakThis.Get();
		if (!Screen) return;
		if (bSuccess) Screen->OnAuthSuccess();
		else Screen->OnAuthFailure();
	});
}

void USeagullTitleScreen::OnGoogleClicked()
{
	// The SDK's SignInGoogle(GoogleAuthCode, RedirectUri, ...) requires a pre-obtained
	// OAuth authorization code and provides no code-acquisition/browser flow, so a
	// desktop example cannot complete it. Surface the limitation directly instead of
	// firing a network call that is guaranteed to fail server-side.
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("Google sign-in is not available on this platform")));
	}
	UE_LOG(LogSeagullStorm, Warning, TEXT("Google sign-in not available on this platform"));
}

void USeagullTitleScreen::OnAppleClicked()
{
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	UHorizonSubsystem* Subsystem = GI->GetHorizonSubsystem();
	if (Subsystem && Subsystem->Auth)
	{
		// Drop-in Apple flow -- native sheet on iOS, no-op fallback on other platforms.
		// On non-iOS the SDK logs a warning and reports failure, which we surface to the user.
		TWeakObjectPtr<USeagullTitleScreen> WeakThis(this);
		Subsystem->Auth->SignInWithApple(FOnAuthComplete::CreateLambda(
			[WeakThis](bool bSuccess)
			{
				USeagullTitleScreen* Screen = WeakThis.Get();
				if (!Screen) return;
				if (bSuccess)
				{
					Screen->OnAuthSuccess();
				}
				else
				{
					if (Screen->StatusText)
					{
						Screen->StatusText->SetText(FText::FromString(TEXT("Apple sign-in is not available on this platform")));
					}
					else
					{
						UE_LOG(LogSeagullStorm, Warning, TEXT("Apple sign-in not available on this platform"));
					}
				}
			}));
	}
	else
	{
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(TEXT("Apple sign-in is not available on this platform")));
		}
		else
		{
			UE_LOG(LogSeagullStorm, Warning, TEXT("Apple sign-in not available on this platform"));
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
	// Surface the failure on screen -- with the placeholder API key the guest
	// path is the first thing that fails, and a silent title screen looks dead.
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("Sign-in failed. Check your API key and connection.")));
	}
	UE_LOG(LogSeagullStorm, Warning, TEXT("Authentication failed"));
}
