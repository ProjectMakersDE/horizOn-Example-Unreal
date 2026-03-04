#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/SeagullTypes.h"
#include "SeagullStormGameMode.generated.h"

class USeagullGameInstance;
class USeagullHorizonManager;
class USeagullAudioManager;
class USeagullEnemySpawner;
class USeagullGameOverScreen;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScreenChanged, ESeagullGameScreen, NewScreen);

UCLASS()
class ASeagullStormGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASeagullStormGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void SwitchToScreen(ESeagullGameScreen NewScreen);

	UFUNCTION()
	void StartRun();

	UFUNCTION()
	void EndRun(bool bPlayerDied);

	UFUNCTION()
	void ReturnToHub();

	ESeagullGameScreen GetCurrentScreen() const { return CurrentScreen; }

	UPROPERTY(BlueprintAssignable)
	FOnScreenChanged OnScreenChanged;

	UPROPERTY()
	USeagullAudioManager* AudioManager = nullptr;

	UPROPERTY()
	USeagullEnemySpawner* EnemySpawner = nullptr;

	// Cached for async rank update after score submission
	UPROPERTY()
	USeagullGameOverScreen* CachedGameOverWidget = nullptr;

private:
	bool bCrashCaptureStarted = false;
	ESeagullGameScreen CurrentScreen = ESeagullGameScreen::Title;
	float SurvivalScoreAccumulator = 0.f;

	USeagullGameInstance* GetSeagullGameInstance() const;
	USeagullHorizonManager* GetHorizonManager() const;

	void CleanupRunActors();

	UFUNCTION()
	void OnLevelUpTriggered();
};
