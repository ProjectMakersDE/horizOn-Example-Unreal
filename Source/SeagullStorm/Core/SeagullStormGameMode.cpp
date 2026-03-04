#include "Core/SeagullStormGameMode.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameState.h"
#include "Core/SeagullPlayerController.h"
#include "Player/SeagullPlayerPawn.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Audio/SeagullAudioManager.h"
#include "Enemies/SeagullEnemySpawner.h"
#include "Map/SeagullMapGenerator.h"
#include "Weapons/SeagullWeaponManager.h"
#include "Weapons/SeagullWeaponBase.h"
#include "Weapons/SeagullWeapon_Feather.h"
#include "Weapons/SeagullWeapon_Screech.h"
#include "Weapons/SeagullWeapon_Dive.h"
#include "Weapons/SeagullWeapon_Gust.h"
#include "Data/SeagullConfigCache.h"
#include "UI/SeagullGameOverScreen.h"
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
	AudioManager->Initialize();

	// Create enemy spawner
	EnemySpawner = NewObject<USeagullEnemySpawner>(this);

	// Connect to horizOn server
	USeagullHorizonManager* HM = GetHorizonManager();
	if (HM)
	{
		HM->ConnectToServer();

		// Start crash capture once at app launch
		if (!bCrashCaptureStarted)
		{
			HM->StartCrashCapture();
			bCrashCaptureStarted = true;
		}
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

			// Add survival time to score (Fix 12: 1 point per second)
			SurvivalScoreAccumulator += DeltaTime;
			if (SurvivalScoreAccumulator >= 1.0f)
			{
				int32 TimePoints = static_cast<int32>(SurvivalScoreAccumulator);
				GS->AddScore(TimePoints);
				SurvivalScoreAccumulator -= static_cast<float>(TimePoints);
			}

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
						// Switch to boss music
						if (AudioManager && AudioManager->MusicBoss)
						{
							AudioManager->CrossfadeToTrack(AudioManager->MusicBoss, GetWorld());
						}
					}
					else
					{
						EndRun(false);
					}
				}
			}
		}

		// Tick enemy spawner — stop spawning after timer expires / boss phase
		if (EnemySpawner && GS && !GS->bBossSpawned && !GS->IsTimerExpired())
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

	// Music transitions
	if (AudioManager)
	{
		switch (NewScreen)
		{
			case ESeagullGameScreen::Title:
			case ESeagullGameScreen::Hub:
				AudioManager->CrossfadeToTrack(AudioManager->MusicMenu, GetWorld());
				break;
			case ESeagullGameScreen::Run:
				AudioManager->CrossfadeToTrack(AudioManager->MusicBattle, GetWorld());
				break;
			case ESeagullGameScreen::GameOver:
				AudioManager->CrossfadeToTrack(AudioManager->MusicMenu, GetWorld());
				if (AudioManager->SFX_GameOver)
				{
					AudioManager->PlaySFX(AudioManager->SFX_GameOver, GetWorld());
				}
				break;
		}
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
	USeagullGameInstance* GI = GetSeagullGameInstance();

	// Initialize game state
	ASeagullStormGameState* GS = GetGameState<ASeagullStormGameState>();
	if (GS)
	{
		GS->InitRun(GI ? GI->GetConfigCache() : nullptr);

		// Bind level-up trigger (Fix 10)
		GS->OnTriggerLevelUpChoices.AddDynamic(this, &ASeagullStormGameMode::OnLevelUpTriggered);
	}

	// Reset enemy spawner
	if (EnemySpawner)
	{
		EnemySpawner->Initialize(GI ? GI->GetConfigCache() : nullptr);
	}

	// Spawn map generator (Fix 8)
	if (GetWorld())
	{
		FActorSpawnParameters MapParams;
		MapParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<ASeagullMapGenerator>(ASeagullMapGenerator::StaticClass(),
			FVector::ZeroVector, FRotator::ZeroRotator, MapParams);
	}

	// Spawn player pawn and possess (Fix 8)
	if (GetWorld())
	{
		FActorSpawnParameters PawnParams;
		PawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASeagullPlayerPawn* Pawn = GetWorld()->SpawnActor<ASeagullPlayerPawn>(
			ASeagullPlayerPawn::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, PawnParams);

		if (Pawn)
		{
			ASeagullPlayerController* PC = Cast<ASeagullPlayerController>(
				UGameplayStatics::GetPlayerController(GetWorld(), 0));
			if (PC)
			{
				PC->Possess(Pawn);
			}

			// Initialize weapon stats from config (Fix 9)
			if (GI && GI->GetConfigCache() && Pawn->WeaponManager)
			{
				USeagullConfigCache* Config = GI->GetConfigCache();
				USeagullWeaponBase* FeatherBase = Pawn->WeaponManager->GetWeapon(ESeagullWeaponType::Feather);
				if (FeatherBase)
				{
					FeatherBase->SetStats(Config->FeatherStats.Damage, Config->FeatherStats.Cooldown);
					USeagullWeapon_Feather* Feather = Cast<USeagullWeapon_Feather>(FeatherBase);
					if (Feather) Feather->ProjectileCount = Config->FeatherStats.Projectiles;
				}

				USeagullWeaponBase* ScreechBase = Pawn->WeaponManager->GetWeapon(ESeagullWeaponType::Screech);
				if (ScreechBase)
				{
					ScreechBase->SetStats(Config->ScreechStats.Damage, Config->ScreechStats.Cooldown);
					USeagullWeapon_Screech* Screech = Cast<USeagullWeapon_Screech>(ScreechBase);
					if (Screech) Screech->Radius = Config->ScreechStats.Range;
				}

				USeagullWeaponBase* DiveBase = Pawn->WeaponManager->GetWeapon(ESeagullWeaponType::Dive);
				if (DiveBase)
				{
					DiveBase->SetStats(Config->DiveStats.Damage, Config->DiveStats.Cooldown);
					USeagullWeapon_Dive* Dive = Cast<USeagullWeapon_Dive>(DiveBase);
					if (Dive) Dive->Range = Config->DiveStats.Range;
				}

				USeagullWeaponBase* GustBase = Pawn->WeaponManager->GetWeapon(ESeagullWeaponType::Gust);
				if (GustBase)
				{
					GustBase->SetStats(Config->GustStats.Damage, Config->GustStats.Cooldown);
					USeagullWeapon_Gust* Gust = Cast<USeagullWeapon_Gust>(GustBase);
					if (Gust) Gust->KnockbackForce = Config->GustStats.Knockback;
				}

				// Store config in WeaponManager for mid-run unlocks
				Pawn->WeaponManager->ConfigCacheRef = Config;
			}
		}
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
			// Submit score first, then fetch rank in callback (Fix 5)
			HM->SubmitScore(static_cast<int64>(GS->CurrentScore), [this, HM](bool bScoreSuccess)
			{
				if (bScoreSuccess)
				{
					// After score is submitted, fetch rank
					HM->GetRank([this](bool bRankSuccess, const FHorizonLeaderboardEntry& Entry)
					{
						if (bRankSuccess && CachedGameOverWidget)
						{
							CachedGameOverWidget->SetRank(Entry.Position);
						}
					});
				}
				else if (HM)
				{
					HM->RecordException(
						TEXT("SubmitScore failed"),
						TEXT("SubmitScore returned bSuccess=false at EndRun"));
				}
			});

			HM->SaveData(GI->SaveData.ToJsonString(), [HM](bool bSaveSuccess)
			{
				if (!bSaveSuccess && HM)
				{
					HM->RecordException(
						TEXT("SaveData failed"),
						TEXT("CloudSave SaveData returned bSuccess=false at EndRun"));
				}
			});

			// User log
			FString LogMsg = FString::Printf(
				TEXT("Run ended | Waves: %d | Level: %d | Score: %d | Duration: %dm%ds | Upgrades: speed:%d,dmg:%d,hp:%d,magnet:%d | Coins earned: %d"),
				GS->CurrentWave, GS->CurrentLevel, GS->CurrentScore,
				static_cast<int32>(GS->RunStats.Duration) / 60,
				static_cast<int32>(GS->RunStats.Duration) % 60,
				GI->SaveData.GetUpgradeLevel(TEXT("speed")),
				GI->SaveData.GetUpgradeLevel(TEXT("damage")),
				GI->SaveData.GetUpgradeLevel(TEXT("hp")),
				GI->SaveData.GetUpgradeLevel(TEXT("magnet")),
				CoinsEarned);
			HM->LogInfo(LogMsg);
		}
	}

	// Mark run as inactive so timer/spawner stop
	if (GS)
	{
		GS->bRunActive = false;
	}

	SurvivalScoreAccumulator = 0.f;
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

void ASeagullStormGameMode::OnLevelUpTriggered()
{
	ASeagullStormGameState* GS = GetGameState<ASeagullStormGameState>();
	if (!GS) return;

	TArray<FSeagullLevelUpChoice> Choices = GS->GetLevelUpChoices(3);
	if (Choices.Num() == 0) return;

	ASeagullPlayerController* PC = Cast<ASeagullPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC)
	{
		PC->ShowLevelUpOverlay(Choices);
	}

	// Breadcrumb for level-up event (Fix 11)
	USeagullHorizonManager* HM = GetHorizonManager();
	if (HM)
	{
		HM->RecordBreadcrumb(TEXT("state"), FString::Printf(TEXT("level_%d"), GS->CurrentLevel));
		HM->SetCrashCustomKey(TEXT("level"), FString::FromInt(GS->CurrentLevel));
	}

	// Play level-up SFX (Fix 6)
	if (AudioManager && AudioManager->SFX_LevelUp)
	{
		AudioManager->PlaySFX(AudioManager->SFX_LevelUp, GetWorld());
	}
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
				Actor->Tags.Contains(TEXT("PlayerPawn")) ||
				Actor->IsA<ASeagullMapGenerator>()))
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
