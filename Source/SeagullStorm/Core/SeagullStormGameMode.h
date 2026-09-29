#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/SeagullTypes.h"
#include "Horizon/SeagullInputLog.h"
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

	// Records the picked level-up card in the run's input log (Validated Actions).
	void RecordLevelUpChoice(int32 ChoiceIndex);

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

	// --- Validated Actions ---
	// Leaderboard the run tickets are bound to: the game's default board.
	static const TCHAR* ValidatedLeaderboardKey;

	// Input log of the current run (seed plus inputs), hashed and submitted at game over.
	FSeagullInputLog InputLog;

	// Counts runs so a late ticket answer from an earlier run is ignored.
	int32 RunSerial = 0;

	// True once this run's ticket arrived.
	bool bValidatedTicketReady = false;

	void BeginValidatedRun();
	void SubmitRunScore(USeagullHorizonManager* HM, int32 Score, int32 Wave, int32 CoinsEarned, float RunSeconds);
	void ShowScoreStatus(const FString& Message, bool bIsError);

	UFUNCTION()
	void OnLevelUpTriggered();
};
