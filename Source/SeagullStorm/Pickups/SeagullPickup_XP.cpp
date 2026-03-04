#include "Pickups/SeagullPickup_XP.h"
#include "Player/SeagullPlayerPawn.h"
#include "Player/SeagullXPComponent.h"
#include "Core/SeagullStormGameState.h"
#include "Core/SeagullStormGameMode.h"
#include "Audio/SeagullAudioManager.h"
#include "Kismet/GameplayStatics.h"

ASeagullPickup_XP::ASeagullPickup_XP()
{
	PickupType = ESeagullPickupType::XP;
}

void ASeagullPickup_XP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Drift toward player if within magnet radius
	ASeagullPlayerPawn* Player = Cast<ASeagullPlayerPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (!Player) return;

	float MagnetRadius = Player->XPComponent ? Player->XPComponent->PickupRadius : 50.f;
	float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

	if (Dist < MagnetRadius && Dist > 5.f)
	{
		FVector Dir = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		float Speed = 200.f;
		SetActorLocation(GetActorLocation() + Dir * Speed * DeltaTime);
	}
}

void ASeagullPickup_XP::OnPickedUp(ASeagullPlayerPawn* Player)
{
	if (Player)
	{
		// Add XP to game state
		ASeagullStormGameState* GS = GetWorld()->GetGameState<ASeagullStormGameState>();
		if (GS)
		{
			GS->AddXP(XPAmount);
			GS->AddScore(XPAmount);
		}

		// Play pickup SFX
		UWorld* World = GetWorld();
		if (World)
		{
			ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(World->GetAuthGameMode());
			if (GM && GM->AudioManager && GM->AudioManager->SFX_PickupXP)
			{
				GM->AudioManager->PlaySFX(GM->AudioManager->SFX_PickupXP, World);
			}
		}
	}

	Super::OnPickedUp(Player);
}
