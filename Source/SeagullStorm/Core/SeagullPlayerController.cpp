#include "Core/SeagullPlayerController.h"
#include "Core/SeagullStormGameMode.h"
#include "UI/SeagullTitleScreen.h"
#include "UI/SeagullMainHub.h"
#include "UI/SeagullHUD.h"
#include "UI/SeagullGameOverScreen.h"
#include "UI/SeagullPauseMenu.h"
#include "UI/SeagullLevelUpOverlay.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "SeagullStorm.h"

ASeagullPlayerController::ASeagullPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ASeagullPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Create input actions programmatically
	MoveAction = NewObject<UInputAction>(this);
	MoveAction->ValueType = EInputActionValueType::Axis2D;

	PauseAction = NewObject<UInputAction>(this);
	PauseAction->ValueType = EInputActionValueType::Boolean;

	// Create and register mapping context
	InputMappingContext = NewObject<UInputMappingContext>(this);

	// WASD bindings
	FEnhancedActionKeyMapping& MoveUpMap = InputMappingContext->MapKey(MoveAction, EKeys::W);
	UInputModifierSwizzleAxis* SwizzleUp = NewObject<UInputModifierSwizzleAxis>(this);
	SwizzleUp->Order = EInputAxisSwizzle::YXZ;
	MoveUpMap.Modifiers.Add(SwizzleUp);

	FEnhancedActionKeyMapping& MoveDownMap = InputMappingContext->MapKey(MoveAction, EKeys::S);
	UInputModifierSwizzleAxis* SwizzleDown = NewObject<UInputModifierSwizzleAxis>(this);
	SwizzleDown->Order = EInputAxisSwizzle::YXZ;
	MoveDownMap.Modifiers.Add(SwizzleDown);
	UInputModifierNegate* NegateDown = NewObject<UInputModifierNegate>(this);
	MoveDownMap.Modifiers.Add(NegateDown);

	InputMappingContext->MapKey(MoveAction, EKeys::D);

	FEnhancedActionKeyMapping& MoveLeftMap = InputMappingContext->MapKey(MoveAction, EKeys::A);
	UInputModifierNegate* NegateLeft = NewObject<UInputModifierNegate>(this);
	MoveLeftMap.Modifiers.Add(NegateLeft);

	// Escape for pause
	InputMappingContext->MapKey(PauseAction, EKeys::Escape);

	// Register
	UEnhancedInputLocalPlayerSubsystem* EIS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (EIS)
	{
		EIS->AddMappingContext(InputMappingContext, 0);
	}
}

void ASeagullPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (EIC)
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASeagullPlayerController::OnMoveInput);
			EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASeagullPlayerController::OnMoveInput);
		}
		if (PauseAction)
		{
			EIC->BindAction(PauseAction, ETriggerEvent::Started, this, &ASeagullPlayerController::OnPauseInput);
		}
	}
}

void ASeagullPlayerController::OnMoveInput(const FInputActionValue& Value)
{
	CachedMoveInput = Value.Get<FVector2D>();
}

void ASeagullPlayerController::OnPauseInput()
{
	ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(GetWorld()->GetAuthGameMode());
	if (GM && GM->GetCurrentScreen() == ESeagullGameScreen::Run)
	{
		TogglePause();
	}
}

void ASeagullPlayerController::OnScreenChanged(ESeagullGameScreen NewScreen)
{
	ClearAllWidgets();

	switch (NewScreen)
	{
		case ESeagullGameScreen::Title:
			TitleWidget = CreateWidget<USeagullTitleScreen>(this);
			if (TitleWidget) TitleWidget->AddToViewport();
			SetInputMode(FInputModeUIOnly());
			break;

		case ESeagullGameScreen::Hub:
			HubWidget = CreateWidget<USeagullMainHub>(this);
			if (HubWidget) HubWidget->AddToViewport();
			SetInputMode(FInputModeUIOnly());
			break;

		case ESeagullGameScreen::Run:
			HUDWidget = CreateWidget<USeagullHUD>(this);
			if (HUDWidget) HUDWidget->AddToViewport();
			SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
			break;

		case ESeagullGameScreen::GameOver:
			GameOverWidget = CreateWidget<USeagullGameOverScreen>(this);
			if (GameOverWidget) GameOverWidget->AddToViewport();
			SetInputMode(FInputModeUIOnly());
			break;
	}
}

void ASeagullPlayerController::ShowLevelUpOverlay(const TArray<FSeagullLevelUpChoice>& Choices)
{
	if (!LevelUpWidget)
	{
		LevelUpWidget = CreateWidget<USeagullLevelUpOverlay>(this);
	}
	if (LevelUpWidget)
	{
		LevelUpWidget->SetChoices(Choices);
		LevelUpWidget->AddToViewport(100);
		SetPause(true);
	}
}

void ASeagullPlayerController::HideLevelUpOverlay()
{
	if (LevelUpWidget)
	{
		LevelUpWidget->RemoveFromParent();
	}
	SetPause(false);
}

void ASeagullPlayerController::TogglePause()
{
	bIsPaused = !bIsPaused;

	if (bIsPaused)
	{
		PauseWidget = CreateWidget<USeagullPauseMenu>(this);
		if (PauseWidget) PauseWidget->AddToViewport(100);
		SetPause(true);
	}
	else
	{
		if (PauseWidget)
		{
			PauseWidget->RemoveFromParent();
			PauseWidget = nullptr;
		}
		SetPause(false);
	}
}

void ASeagullPlayerController::ClearAllWidgets()
{
	if (TitleWidget) { TitleWidget->RemoveFromParent(); TitleWidget = nullptr; }
	if (HubWidget) { HubWidget->RemoveFromParent(); HubWidget = nullptr; }
	if (HUDWidget) { HUDWidget->RemoveFromParent(); HUDWidget = nullptr; }
	if (GameOverWidget) { GameOverWidget->RemoveFromParent(); GameOverWidget = nullptr; }
	if (PauseWidget) { PauseWidget->RemoveFromParent(); PauseWidget = nullptr; }
	if (LevelUpWidget) { LevelUpWidget->RemoveFromParent(); LevelUpWidget = nullptr; }
	bIsPaused = false;
}
