#include "Core/SeagullStormGameState.h"
#include "Data/SeagullConfigCache.h"
#include "SeagullStorm.h"
#include "Kismet/GameplayStatics.h"

void ASeagullStormGameState::InitRun(USeagullConfigCache* Config)
{
	ConfigCacheRef = Config;

	CurrentWave = 0;
	CurrentScore = 0;
	CurrentLevel = 1;
	KillCount = 0;
	TotalXPCollected = 0;
	CurrentXP = 0;
	bBossSpawned = false;
	bRunActive = true;

	TimeRemaining = Config ? static_cast<float>(Config->RunDurationSeconds) : 180.f;
	XPToNextLevel = CalculateXPToNextLevel();

	RunStats = FSeagullRunStats();
	RunStartTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	// Start with feather weapon
	ActiveWeapons.Empty();
	ActiveWeapons.Add(ESeagullWeaponType::Feather);
	WeaponLevels.Empty();
	WeaponLevels.Add(ESeagullWeaponType::Feather, 1);
}

void ASeagullStormGameState::TickTimer(float DeltaTime)
{
	if (!bRunActive) return;

	TimeRemaining -= DeltaTime;
	RunStats.Duration += DeltaTime;

	if (TimeRemaining <= 0.f)
	{
		TimeRemaining = 0.f;
		OnTimerExpired.Broadcast();
	}
}

void ASeagullStormGameState::AddScore(int32 Amount)
{
	CurrentScore += Amount;
	RunStats.Score = CurrentScore;
	OnScoreChanged.Broadcast(CurrentScore);
}

void ASeagullStormGameState::AddKill()
{
	KillCount++;
	RunStats.Kills = KillCount;
	AddScore(10); // Base score per kill
}

void ASeagullStormGameState::AddXP(int32 Amount)
{
	CurrentXP += Amount;
	TotalXPCollected += Amount;
	RunStats.XPCollected = TotalXPCollected;

	// Check for level up
	while (CurrentXP >= XPToNextLevel)
	{
		CurrentXP -= XPToNextLevel;
		CurrentLevel++;
		RunStats.LevelReached = CurrentLevel;
		XPToNextLevel = CalculateXPToNextLevel();
		OnLevelUp.Broadcast(CurrentLevel);
		OnTriggerLevelUpChoices.Broadcast();

		UE_LOG(LogSeagullStorm, Log, TEXT("Level up! Now level %d"), CurrentLevel);
	}
}

void ASeagullStormGameState::AdvanceWave()
{
	CurrentWave++;
	RunStats.WavesReached = CurrentWave;
	OnWaveChanged.Broadcast(CurrentWave);
	UE_LOG(LogSeagullStorm, Log, TEXT("Wave %d started"), CurrentWave);
}

TArray<FSeagullLevelUpChoice> ASeagullStormGameState::GetLevelUpChoices(int32 Count) const
{
	TArray<FSeagullLevelUpChoice> Result;
	if (!ConfigCacheRef || ConfigCacheRef->LevelUpPool.Num() == 0) return Result;

	// Weighted random selection
	TArray<FSeagullLevelUpChoice> Pool = ConfigCacheRef->LevelUpPool;

	// Filter out weapons already at max or already owned (for "new" weapons)
	TArray<FSeagullLevelUpChoice> FilteredPool;
	for (const FSeagullLevelUpChoice& Choice : Pool)
	{
		if (Choice.Type == TEXT("weapon_new"))
		{
			// Check if already unlocked
			ESeagullWeaponType WType = ESeagullWeaponType::Feather;
			if (Choice.Id == TEXT("screech_new")) WType = ESeagullWeaponType::Screech;
			else if (Choice.Id == TEXT("dive_new")) WType = ESeagullWeaponType::Dive;
			else if (Choice.Id == TEXT("gust_new")) WType = ESeagullWeaponType::Gust;

			if (!ActiveWeapons.Contains(WType))
			{
				FilteredPool.Add(Choice);
			}
		}
		else
		{
			FilteredPool.Add(Choice);
		}
	}

	if (FilteredPool.Num() == 0) FilteredPool = Pool;

	// Calculate total weight
	int32 TotalWeight = 0;
	for (const auto& C : FilteredPool) TotalWeight += C.Weight;

	// Pick N unique choices
	TArray<int32> UsedIndices;
	for (int32 i = 0; i < Count && UsedIndices.Num() < FilteredPool.Num(); i++)
	{
		int32 Roll = FMath::RandRange(0, TotalWeight - 1);
		int32 Accumulated = 0;
		for (int32 j = 0; j < FilteredPool.Num(); j++)
		{
			if (UsedIndices.Contains(j)) continue;
			Accumulated += FilteredPool[j].Weight;
			if (Roll < Accumulated)
			{
				Result.Add(FilteredPool[j]);
				UsedIndices.Add(j);
				TotalWeight -= FilteredPool[j].Weight;
				break;
			}
		}
	}

	return Result;
}

void ASeagullStormGameState::ApplyLevelUpChoice(const FSeagullLevelUpChoice& Choice)
{
	if (Choice.Type == TEXT("weapon_new"))
	{
		ESeagullWeaponType WType = ESeagullWeaponType::Feather;
		if (Choice.Id == TEXT("screech_new")) WType = ESeagullWeaponType::Screech;
		else if (Choice.Id == TEXT("dive_new")) WType = ESeagullWeaponType::Dive;
		else if (Choice.Id == TEXT("gust_new")) WType = ESeagullWeaponType::Gust;

		if (!ActiveWeapons.Contains(WType))
		{
			ActiveWeapons.Add(WType);
			WeaponLevels.Add(WType, 1);
		}
	}
	else if (Choice.Type == TEXT("weapon_upgrade"))
	{
		ESeagullWeaponType WType = ESeagullWeaponType::Feather;
		if (Choice.Id.Contains(TEXT("feather"))) WType = ESeagullWeaponType::Feather;

		int32* Level = WeaponLevels.Find(WType);
		if (Level) (*Level)++;
	}
	// stat_boost choices are handled by the pawn
}

int32 ASeagullStormGameState::CalculateXPToNextLevel() const
{
	int32 Base = 50;
	float Curve = 1.4f;
	if (ConfigCacheRef)
	{
		Base = ConfigCacheRef->XPPerKillBase * 5; // Base XP = 50
		Curve = ConfigCacheRef->XPLevelCurve;
	}
	return static_cast<int32>(Base * FMath::Pow(static_cast<float>(CurrentLevel), Curve));
}
