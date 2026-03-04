#include "Core/SeagullStormGameMode.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameState.h"
#include "Core/SeagullPlayerController.h"
#include "Player/SeagullPlayerPawn.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Audio/SeagullAudioManager.h"
#include "Enemies/SeagullEnemySpawner.h"
#include "SeagullStorm.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ASeagullStormGameMode::ASeagullStormGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultPawnClass = nullptr; // No pawn on title/hub
	PlayerControllerClass = ASeagullPlayerController::StaticClass();
	GameStateClass = ASeagullStormGameState::StaticClass();
}

void ASeagullStormGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Create audio manager
	AudioManager = NewObject<USeagullAudioManager>(this);

	// Create enemy spawner
	EnemySpawner = NewObject<USeagullEnemySpawner>(this);

	// Connect to horizOn server
	USeagullHorizonManager* HM = GetHorizonManager();
	if (HM)
	{
		HM->ConnectToServer();
	}

	// Try session restore
	if (HM)
	{
		HM->RestoreSession([this](bool bSuccess)
		{
			if (bSuccess)
			{
				UE_LOG(LogSeagullStorm, Log, TEXT("Session restored, going to Hub"));
				SwitchToScreen(ESeagullGameScreen::Hub);
			}
			else
			{
				UE_LOG(LogSeagullStorm, Log, TEXT("No session to restore, showing Title"));
				SwitchToScreen(ESeagullGameScreen::Title);
			}
		});
	}
	else
	{
		SwitchToScreen(ESeagullGameScreen::Title);
	}
}

void ASeagullStormGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentScreen == ESeagullGameScreen::Run)
	{
		ASeagullStormGameState* GS = GetGameState<ASeagullStormGameState>();
		if (GS)
		{
			GS->TickTimer(DeltaTime);

			if (GS->IsTimerExpired())
			{
				// Spawn boss or end run
				if (EnemySpawner && !GS->bBossSpawned)
				{
					GS->bBossSpawned = true;
					USeagullGameInstance* GI = GetSeagullGameInstance();
					bool bBossEnabled = true;
					if (GI && GI->GetConfigCache())
					{
						bBossEnabled = GI->GetConfigCache()->bBossWaveEnabled;
					}
					if (bBossEnabled)
					{
						EnemySpawner->SpawnBoss(GetWorld());
					}
					else
					{
						EndRun(false);
					}
				}
			}
		}

		// Tick enemy spawner
		if (EnemySpawner)
		{
			EnemySpawner->Tick(DeltaTime, GetWorld());
		}
	}
}

void ASeagullStormGameMode::SwitchToScreen(ESeagullGameScreen NewScreen)
{
	ESeagullGameScreen OldScreen = CurrentScreen;
	CurrentScreen = NewScreen;

	UE_LOG(LogSeagullStorm, Log, TEXT("Screen transition: %d -> %d"), static_cast<int32>(OldScreen), static_cast<int32>(NewScreen));

	// Notify controller to swap UI
	ASeagullPlayerController* PC = Cast<ASeagullPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC)
	{
		PC->OnScreenChanged(NewScreen);
	}

	// Record breadcrumb
	USeagullHorizonManager* HM = GetHorizonManager();
	if (HM)
	{
		FString ScreenName;
		switch (NewScreen)
		{
			case ESeagullGameScreen::Title: ScreenName = TEXT("title"); break;
			case ESeagullGameScreen::Hub: ScreenName = TEXT("hub"); break;
			case ESeagullGameScreen::Run: ScreenName = TEXT("run"); break;
			case ESeagullGameScreen::GameOver: ScreenName = TEXT("game_over"); break;
		}
		HM->RecordBreadcrumb(TEXT("navigation"), FString::Printf(TEXT("entered_%s"), *ScreenName));
	}

	OnScreenChanged.Broadcast(NewScreen);
}

void ASeagullStormGameMode::StartRun()
{
	// Initialize game state
	ASeagullStormGameState* GS = GetGameState<ASeagullStormGameState>();
	if (GS)
	{
		USeagullGameInstance* GI = GetSeagullGameInstance();
		GS->InitRun(GI ? GI->GetConfigCache() : nullptr);
	}

	// Reset enemy spawner
	if (EnemySpawner)
	{
		USeagullGameInstance* GI = GetSeagullGameInstance();
		EnemySpawner->Initialize(GI ? GI->GetConfigCache() : nullptr);
	}

	SwitchToScreen(ESeagullGameScreen::Run);
}

void ASeagullStormGameMode::EndRun(bool bPlayerDied)
{
	ASeagullStormGameState* GS = GetGameState<ASeagullStormGameState>();
	USeagullGameInstance* GI = GetSeagullGameInstance();
	USeagullHorizonManager* HM = GetHorizonManager();

	if (GS && GI)
	{
		// Calculate coins
		int32 CoinDivisor = 10;
		if (GI->GetConfigCache())
		{
			CoinDivisor = FMath::Max(1, GI->GetConfigCache()->CoinDivisor);
		}
		int32 CoinsEarned = GS->CurrentScore / CoinDivisor;
		GS->RunStats.CoinsEarned = CoinsEarned;

		// Update save data
		GI->SaveData.Coins += CoinsEarned;
		GI->SaveData.TotalRuns++;
		if (GS->CurrentScore > GI->SaveData.Highscore)
		{
			GI->SaveData.Highscore = GS->CurrentScore;
		}

		// Track wave-1 deaths
		if (bPlayerDied && GS->CurrentWave <= 1)
		{
			GI->ConsecutiveWave1Deaths++;
			if (GI->ConsecutiveWave1Deaths >= 3 && HM)
			{
				HM->LogWarn(TEXT("Player died in wave 1 three consecutive times - possible balancing issue"));
				GI->ConsecutiveWave1Deaths = 0;
			}
		}
		else
		{
			GI->ConsecutiveWave1Deaths = 0;
		}

		// Submit to horizOn
		if (HM)
		{
			HM->SubmitScore(static_cast<int64>(GS->CurrentScore), [](bool) {});
			HM->SaveData(GI->SaveData.ToJsonString(), [](bool) {});

			// User log
			FString LogMsg = FString::Printf(
				TEXT("Run ended | Waves: %d | Level: %d | Score: %d | Duration: %dm%ds | Upgrades: speed:%d,dmg:%d,hp:%d | Coins earned: %d"),
				GS->CurrentWave, GS->CurrentLevel, GS->CurrentScore,
				static_cast<int32>(GS->RunStats.Duration) / 60,
				static_cast<int32>(GS->RunStats.Duration) % 60,
				GI->SaveData.GetUpgradeLevel(TEXT("speed")),
				GI->SaveData.GetUpgradeLevel(TEXT("damage")),
				GI->SaveData.GetUpgradeLevel(TEXT("hp")),
				CoinsEarned);
			HM->LogInfo(LogMsg);
		}
	}

	CleanupRunActors();
	SwitchToScreen(ESeagullGameScreen::GameOver);
}

void ASeagullStormGameMode::ReturnToHub()
{
	CleanupRunActors();
	SwitchToScreen(ESeagullGameScreen::Hub);
}

USeagullGameInstance* ASeagullStormGameMode::GetSeagullGameInstance() const
{
	return Cast<USeagullGameInstance>(GetGameInstance());
}

USeagullHorizonManager* ASeagullStormGameMode::GetHorizonManager() const
{
	USeagullGameInstance* GI = GetSeagullGameInstance();
	return GI ? GI->GetHorizonManager() : nullptr;
}

void ASeagullStormGameMode::CleanupRunActors()
{
	// Destroy all enemies, pickups, projectiles
	if (GetWorld())
	{
		TArray<AActor*> ToDestroy;
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && (Actor->Tags.Contains(TEXT("Enemy")) ||
				Actor->Tags.Contains(TEXT("Pickup")) ||
				Actor->Tags.Contains(TEXT("Projectile")) ||
				Actor->Tags.Contains(TEXT("PlayerPawn"))))
			{
				ToDestroy.Add(Actor);
			}
		}
		for (AActor* Actor : ToDestroy)
		{
			Actor->Destroy();
		}
	}
}
