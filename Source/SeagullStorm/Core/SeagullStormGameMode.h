#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/SeagullTypes.h"
#include "SeagullStormGameMode.generated.h"

class USeagullGameInstance;
class USeagullHorizonManager;
class USeagullAudioManager;
class USeagullEnemySpawner;

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

private:
	ESeagullGameScreen CurrentScreen = ESeagullGameScreen::Title;

	USeagullGameInstance* GetSeagullGameInstance() const;
	USeagullHorizonManager* GetHorizonManager() const;

	void CleanupRunActors();
};
