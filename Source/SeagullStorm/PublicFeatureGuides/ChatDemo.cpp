#include "HorizonSubsystem.h"
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonChatManager.h"
#include "Models/HorizonChat.h"

// Explicit raw REST read; the managed feed remains separately subscription-gated.
void ReadChatSnapshot(UHorizonSubsystem* SDK, const FString& ChannelId)
{
    if (!SDK || !SDK->Auth || !SDK->Chat) return;
    const FString UserId = SDK->Auth->GetCurrentUser().UserId;
    if (UserId.IsEmpty()) return;
    SDK->Chat->SyncChannel(UserId, ChannelId,
        FOnChatSyncedComplete::CreateLambda([](bool bSuccess,
            const FHorizonChatSynced& Result,
            const FString& Code, const FString& Message)
        {
            if (!bSuccess) return;
            UE_LOG(LogTemp, Log, TEXT("Chat epoch %s: %d retained IDs"),
                *Result.Channel.Epoch, Result.VisibleMessageIds.Num());
            // Explicit snapshot only. Never expose a disconnected managed feed here.
        }));
}
// SDK UMG recipe: Examples/HorizonChatExample.h / AHorizonChatExample.
// WatchChannel(User,Channel) -> confirmed subscription -> OnFeedChanged -> GetFeed(Channel).
// The SDK coalesces event/resync invalidations into one Sync plus a dirty bit; no timer poll.
// SendMessage(User,Channel,OriginalOperationId,OriginalIssuedAt,OriginalText,FOnChatSentComplete).
// Keep the durable project+player original through host/session changes; never replace its ID.
// Native delegates use CreateLambda/BindUObject, never BindDynamic. PHP Chat returns 501.
