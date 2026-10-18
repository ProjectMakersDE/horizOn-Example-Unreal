#include "HorizonSubsystem.h"
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonItemsManager.h"
#include "Models/HorizonItems.h"

// Explicit player actions after restoring the existing matching session.
void ReadInventory(UHorizonSubsystem* SDK)
{
    if (!SDK || !SDK->Auth || !SDK->Auth->IsSignedIn() || !SDK->Items) return;
    const FString UserId = SDK->Auth->GetCurrentUser().UserId;
    SDK->Items->GetInventory(UserId,
        FOnInventoryComplete::CreateLambda([](bool bSuccess,
            const FHorizonInventorySnapshot& Result,
            const FString& Code, const FString& Message)
        {
            if (!bSuccess) return;
            for (const FHorizonItemInstance& Item : Result.Instances)
                UE_LOG(LogTemp, Log, TEXT("Item %s: %s"), *Item.InstanceId, *Item.ItemKey);
            // Keep Result.Revision for a separately reviewed mutation.
        }));
}

void ReadCatalogPage(UHorizonSubsystem* SDK)
{
    if (!SDK || !SDK->Auth || !SDK->Auth->IsSignedIn() || !SDK->Items) return;
    const FString UserId = SDK->Auth->GetCurrentUser().UserId;
    SDK->Items->GetCatalog(UserId,
        FOnItemCatalogComplete::CreateLambda([](bool bSuccess,
            const FHorizonItemCatalogPage& Result,
            const FString& Code, const FString& Message)
        {
            if (!bSuccess) return;
            UE_LOG(LogTemp, Log, TEXT("Catalog revision: %lld"), Result.Revision);
            // Explicit Next keeps Result.Revision and NextAfterKey. No page loop or poll.
        }), TEXT(""), 1);
}
// SDK Examples/HorizonItemsExample.h journals original instance/body/revision before writes.
// Purchase recovery keeps the original Player Values operationId.
// Initial success may include item references; retained replay may return empty applied/items.
// Read the current inventory separately. Never reconstruct earlier item references.
// A failed refresh never authorizes another purchase or a local item grant.
