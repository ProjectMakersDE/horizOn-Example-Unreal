#pragma once

#include "CoreMinimal.h"
#include "SeagullTypes.generated.h"

// --- Game Screen States ---

UENUM(BlueprintType)
enum class ESeagullGameScreen : uint8
{
	Title,
	Hub,
	Run,
	GameOver
};

// --- Weapon Types ---

UENUM(BlueprintType)
enum class ESeagullWeaponType : uint8
{
	Feather,
	Screech,
	Dive,
	Gust
};

// --- Enemy Types ---

UENUM(BlueprintType)
enum class ESeagullEnemyType : uint8
{
	Crab,
	Jellyfish,
	Pirate,
	Boss
};

// --- Pickup Types ---

UENUM(BlueprintType)
enum class ESeagullPickupType : uint8
{
	XP,
	Coin,
	Health
};

// --- Feedback Category ---

UENUM(BlueprintType)
enum class ESeagullFeedbackCategory : uint8
{
	Bug,
	Feature,
	General
};

// --- Level-Up Choice ---

USTRUCT(BlueprintType)
struct FSeagullLevelUpChoice
{
	GENERATED_BODY()

	UPROPERTY()
	FString Id;

	UPROPERTY()
	FString Type; // "weapon_upgrade", "weapon_new", "stat_boost"

	UPROPERTY()
	int32 Weight = 1;

	UPROPERTY()
	FString DisplayName;

	UPROPERTY()
	FString Description;
};

// --- Run Stats (for user log at game over) ---

USTRUCT(BlueprintType)
struct FSeagullRunStats
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Kills = 0;

	UPROPERTY()
	int32 XPCollected = 0;

	UPROPERTY()
	float Duration = 0.f;

	UPROPERTY()
	int32 WavesReached = 0;

	UPROPERTY()
	int32 LevelReached = 0;

	UPROPERTY()
	int32 Score = 0;

	UPROPERTY()
	int32 CoinsEarned = 0;
};

// --- Enemy Stats (from Remote Config) ---

USTRUCT()
struct FSeagullEnemyStats
{
	GENERATED_BODY()

	float Speed = 40.f;
	int32 HP = 30;
	int32 Damage = 10;
	int32 XPDrop = 10;
};

// --- Weapon Stats (from Remote Config) ---

USTRUCT()
struct FSeagullWeaponStats
{
	GENERATED_BODY()

	float Damage = 20.f;
	float Cooldown = 0.8f;
	float Range = 0.f;
	int32 Projectiles = 1;
	float Knockback = 0.f;
};

// --- Color Palette ---

namespace SeagullColors
{
	const FLinearColor Orange       = FLinearColor(0.847f, 0.474f, 0.263f); // #D87943
	const FLinearColor Teal         = FLinearColor(0.322f, 0.459f, 0.459f); // #527575
	const FLinearColor DarkBG       = FLinearColor(0.102f, 0.102f, 0.180f); // #1A1A2E
	const FLinearColor LightText    = FLinearColor(0.933f, 0.933f, 0.933f); // #EEEEEE
	const FLinearColor DarkText     = FLinearColor(0.110f, 0.110f, 0.110f); // #1C1C1C
	const FLinearColor Sand         = FLinearColor(0.949f, 0.824f, 0.663f); // #F2D2A9
	const FLinearColor Water        = FLinearColor(0.231f, 0.490f, 0.847f); // #3B7DD8
	const FLinearColor XPGold       = FLinearColor(1.000f, 0.843f, 0.000f); // #FFD700
	const FLinearColor CrabRed      = FLinearColor(0.878f, 0.357f, 0.294f); // #E05B4B
	const FLinearColor JellyPurple  = FLinearColor(0.608f, 0.349f, 0.714f); // #9B59B6
	const FLinearColor PirateDark   = FLinearColor(0.290f, 0.290f, 0.290f); // #4A4A4A
	const FLinearColor SeagullWhite = FLinearColor(0.961f, 0.961f, 0.941f); // #F5F5F0
}

// --- Game Constants ---

namespace SeagullConstants
{
	constexpr float BaseResolutionWidth = 480.f;
	constexpr float BaseResolutionHeight = 270.f;
	constexpr float RenderScale = 3.f;
	constexpr float InvincibilityDuration = 0.5f;
	constexpr float MusicCrossfadeDuration = 0.5f;
	constexpr float MusicVolume = 0.7f;
	constexpr float SFXVolume = 1.0f;
	constexpr int32 MaxSFXPickupXP = 3;
	constexpr int32 MaxSFXEnemyHit = 5;
	constexpr int32 LeaderboardTopCount = 10;
	constexpr int32 NewsCount = 5;
	constexpr float ArenaWidth = 1200.f;
	constexpr float ArenaHeight = 800.f;
}
