#include "Examples/HorizonMarketplaceQuickstart.h"
#include "Examples/HorizonMarketplaceExample.h"

// Retain Screen and Bind it to the existing subsystem and stable project UUID.
// Display the successful Marketplace->OnResult point response before confirming.
void ReadMarketplaceListing(UHorizonMarketplaceExample* Screen, const FString& ListingId)
{
    if (Screen) UHorizonMarketplaceQuickstart::ReadOneListing(Screen, ListingId);
}

// Call only from the player's explicit Buy confirmation.
bool ConfirmMarketplacePurchase(UHorizonMarketplaceExample* Screen, const FString& ListingId)
{
    return Screen && UHorizonMarketplaceQuickstart::BuyReviewedListing(Screen, ListingId);
}
// true means started, not purchased. Observe Screen->OnAction / LastErrorCode.
// Unknown outcome: RetryPending / RecoverOriginalOperation keep the same original.
// The sample refreshes Inventory, Values and Marketplace before acknowledgement.