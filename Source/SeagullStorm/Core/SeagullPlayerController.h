#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/SeagullTypes.h"
#include "SeagullPlayerController.generated.h"

class USeagullTitleScreen;
class USeagullMainHub;
class USeagullHUD;
class USeagullGameOverScreen;
class USeagullPauseMenu;
class USeagullLevelUpOverlay;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class ASeagullPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASeagullPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void OnScreenChanged(ESeagullGameScreen NewScreen);
	void ShowLevelUpOverlay(const TArray<FSeagullLevelUpChoice>& Choices);
	void HideLevelUpOverlay();

	void TogglePause();

	// Input
	void OnMoveInput(const FInputActionValue& Value);
	void OnPauseInput();

	FVector2D GetMoveInput() const { return CachedMoveInput; }

private:
	void ClearAllWidgets();

	// Widgets
	UPROPERTY()
	USeagullTitleScreen* TitleWidget = nullptr;

	UPROPERTY()
	USeagullMainHub* HubWidget = nullptr;

	UPROPERTY()
	USeagullHUD* HUDWidget = nullptr;

	UPROPERTY()
	USeagullGameOverScreen* GameOverWidget = nullptr;

	UPROPERTY()
	USeagullPauseMenu* PauseWidget = nullptr;

	UPROPERTY()
	USeagullLevelUpOverlay* LevelUpWidget = nullptr;

	// Enhanced Input
	UPROPERTY()
	UInputMappingContext* InputMappingContext = nullptr;

	UPROPERTY()
	UInputAction* MoveAction = nullptr;

	UPROPERTY()
	UInputAction* PauseAction = nullptr;

	FVector2D CachedMoveInput = FVector2D::ZeroVector;
	bool bIsPaused = false;
};
