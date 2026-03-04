#include "Pickups/SeagullPickup_Coin.h"

ASeagullPickup_Coin::ASeagullPickup_Coin()
{
	PickupType = ESeagullPickupType::Coin;
}

void ASeagullPickup_Coin::OnPickedUp(ASeagullPlayerPawn* Player)
{
	// Coins are visual only -- actual coins derived from score at game over
	Super::OnPickedUp(Player);
}
