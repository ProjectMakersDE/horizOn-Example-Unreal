#include "HorizonSubsystem.h"
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonSharedObjectsManager.h"

// Verified SDK Develop API; invoke on an explicit player action.
void ReadWorldMap(UHorizonSubsystem* SDK)
{
    if (!SDK || !SDK->Auth || !SDK->SharedObjects) return;
    const FString UserId = SDK->Auth->GetCurrentUser().UserId;
    FHorizonSharedQuery Query;
    Query.Mode = EHorizonSharedQueryMode::Rectangle;
    Query.MinX = -10; Query.MaxX = 10; Query.MinY = -10; Query.MaxY = 10;
    Query.Limit = 20;
    SDK->SharedObjects->QueryObjects(UserId, TEXT("bases"), Query,
        FOnSharedQueryComplete::CreateLambda([](bool bSuccess,
            const FHorizonSharedQueryResult& Result,
            const FString& Code, const FString& Message)
        {
            if (!bSuccess) return;
            for (const FHorizonSharedObject& Object : Result.Objects)
                UE_LOG(LogTemp, Log, TEXT("Object %s: %d bytes"),
                    *Object.Id, Object.Data.Num());
            // Respect Result.PollAfterSeconds. No auto-poll/page traversal.
        }));
}
// Persist original target, bytes and If-Match before a single mutation.
// Unknown Create: explicit owner-partition/usage reconciliation, no replacement send.
// Bytes are opaque. Trusted backend validation and Inbox delivery are separate.
