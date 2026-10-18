#include "HorizonSubsystem.h"
#include "Engine/GameInstance.h"
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonFriendsManager.h"
#include "Models/HorizonFriends.h"

// Existing SDK/project configuration and authenticated player session required.
// Call this function explicitly after reviewing the example and its data.
void RunReviewedFriendsDemo(UGameInstance* GameInstance)
{
    if (!GameInstance) return;
    auto* Existing = GameInstance->GetSubsystem<UHorizonSubsystem>();
    if (!Existing || !Existing->Auth || !Existing->Auth->IsSignedIn()) return;
    const auto GetGameInstance = [GameInstance]() { return GameInstance; };
    UHorizonSubsystem* SDK = GetGameInstance()->GetSubsystem<UHorizonSubsystem>();
    const FString UserId = SDK->Auth->GetCurrentUser().UserId;
    SDK->Friends->GetFriends(UserId, FOnFriendsComplete::CreateLambda(
        [](bool bSuccess, const FHorizonFriendsSnapshot& Snapshot,
            const FString& Code, const FString& Message)
        {
            if (!bSuccess) { UE_LOG(LogTemp, Warning, TEXT("Friends: %s"), *Code); return; }
            UE_LOG(LogTemp, Log, TEXT("Public friend code: %s"), *Snapshot.FriendCode);
            for (const auto& Friend : Snapshot.Friends)
                UE_LOG(LogTemp, Log, TEXT("%s, last login: %s"), *Friend.UserId,
                    Friend.bHasLastActiveAt ? *Friend.LastActiveAt : TEXT("unknown"));
        }));
    // On an explicit player action, retain the reviewed revision and original request identity:
    auto ApplyReviewed = [SDK, UserId](const FString& TargetUserId, int64 ExpectedRevision,
        const FString& RequestId)
    {
        SDK->Friends->AcceptRequest(UserId, TargetUserId, ExpectedRevision, RequestId,
            FOnFriendsComplete::CreateLambda([](bool bSuccess, const FHorizonFriendsSnapshot& Snapshot,
                const FString& Code, const FString& Message)
            {
                if (!bSuccess) { UE_LOG(LogTemp, Warning, TEXT("Refresh and reconcile: %s"), *Code); return; }
                // Replace the entire displayed snapshot. Never automatically retry with a new revision.
            }));
    };
    // SendRequest/RemoveFriend/BlockPlayer/UnblockPlayer(UserId,TargetUserId,ExpectedRevision,Delegate).
    // AcceptRequest/DeclineRequest/CancelRequest also take the original RequestId before Delegate.
    // FindPlayer(UserId,ExactUuidOrHCode,FOnFriendLookupComplete), never name/email discovery.
    // LastActiveAt with bHasLastActiveAt=false is unknown; never present it as online.
    // Refresh on explicit lifecycle/invalidation events, with no hidden polling timer.
}
