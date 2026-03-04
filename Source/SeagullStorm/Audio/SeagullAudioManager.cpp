#include "Audio/SeagullAudioManager.h"
#include "Core/SeagullTypes.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"

void USeagullAudioManager::PlayMusic(USoundWave* Track, UWorld* World)
{
	if (!Track || !World) return;
	if (CurrentMusicTrack == Track) return;

	StopMusic();

	MusicComponent = UGameplayStatics::CreateSound2D(World, Track, SeagullConstants::MusicVolume, 1.0f, 0.f, nullptr, false, true);
	if (MusicComponent)
	{
		MusicComponent->Play();
	}
	CurrentMusicTrack = Track;
}

void USeagullAudioManager::StopMusic()
{
	if (MusicComponent)
	{
		MusicComponent->Stop();
		MusicComponent = nullptr;
	}
	CurrentMusicTrack = nullptr;
}

void USeagullAudioManager::PlaySFX(USoundWave* Sound, UWorld* World)
{
	if (!Sound || !World) return;

	// Polyphony check
	int32 Max = GetMaxPolyphony(Sound);
	int32* Count = ActiveSFXCount.Find(Sound);
	if (Count && *Count >= Max) return;

	UGameplayStatics::PlaySound2D(World, Sound, SeagullConstants::SFXVolume);

	// Track active SFX (simplified -- no callback on completion)
	if (Count)
	{
		(*Count)++;
	}
	else
	{
		ActiveSFXCount.Add(Sound, 1);
	}

	// Reset count after a short time (simplified approach)
	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, [this, Sound]()
	{
		int32* C = ActiveSFXCount.Find(Sound);
		if (C && *C > 0) (*C)--;
	}, 0.3f, false);
}

void USeagullAudioManager::CrossfadeToTrack(USoundWave* NewTrack, UWorld* World)
{
	// Simplified crossfade -- just switch tracks
	PlayMusic(NewTrack, World);
}

int32 USeagullAudioManager::GetMaxPolyphony(USoundWave* Sound) const
{
	if (Sound == SFX_PickupXP) return SeagullConstants::MaxSFXPickupXP;
	if (Sound == SFX_EnemyHit) return SeagullConstants::MaxSFXEnemyHit;
	return 10; // Default max
}
