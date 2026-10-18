#include "HorizonSubsystem.h"
#include "Engine/GameInstance.h"
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonInboxManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

// Existing SDK/project configuration and authenticated player session required.
// Call this function explicitly after reviewing the example and its data.
void RunReviewedInboxDemo(UGameInstance* GameInstance)
{
    if (!GameInstance) return;
    auto* Existing = GameInstance->GetSubsystem<UHorizonSubsystem>();
    if (!Existing || !Existing->Auth || !Existing->Auth->IsSignedIn()) return;
    const auto GetGameInstance = [GameInstance]() { return GameInstance; };
    auto* SDK = GetGameInstance()->GetSubsystem<UHorizonSubsystem>();
    const FString UserId = SDK->Auth->GetCurrentUser().UserId;
    SDK->Inbox->GetInbox(UserId, FOnInboxComplete::CreateLambda(
        [](bool Success, const FHorizonInboxSnapshot& Snapshot,
           const FString& Code, const FString& Message)
        {
            if (!Success) { UE_LOG(LogTemp, Warning, TEXT("%s"), *Code); return; }
            for (const auto& Entry : Snapshot.Entries)
                UE_LOG(LogTemp, Log, TEXT("%s"), *Entry.TextKey);
            // The SDK keeps full cached entries for unchanged responses.
            // Respect Snapshot.PollAfterSeconds and the game's lifecycle.
        }));
    auto ClaimBusy = MakeShared<bool>(false);
    auto ClaimSelected = [SDK, UserId, ClaimBusy](const FString& EntryId)
    {
        if (*ClaimBusy) return;
        const FString ProjectId = TEXT("YOUR_PROJECT_ID");
        const FString Path = FPaths::ProjectSavedDir() + TEXT("inbox_claim_") + ProjectId + TEXT("_") + UserId + TEXT(".txt");
        FString Pending;
        if (FPaths::FileExists(Path))
        {
            if (!FFileHelper::LoadFileToString(Pending, *Path) || Pending != EntryId) return;
        }
        else
        {
            const auto Current = SDK->Inbox->GetCurrentInbox();
            if (Current.UserId != UserId) return;
            const auto* Selected = Current.Entries.FindByPredicate(
                [&EntryId](const auto& Entry) { return Entry.Id == EntryId; });
            if (!Selected || Selected->bClaimed || Selected->Attachment.Values.IsEmpty()) return;
        }
        // An existing identical pending ID bypasses this new-selection guard for safe replay.
        if (!FFileHelper::SaveStringToFile(EntryId, *Path)) return;
        *ClaimBusy = true;
        SDK->Inbox->Claim(UserId, EntryId, FOnInboxClaimComplete::CreateLambda(
            [Path, EntryId, ClaimBusy](bool Success, const FHorizonInboxClaim& Result,
                   const FString& Code, const FString& Message)
            {
                *ClaimBusy = false;
                if (!Success) { UE_LOG(LogTemp, Warning, TEXT("%s"), *Code); return; }
                // Display Result.Inbox and replace values with Result.PlayerValues.
                // Never apply the reward delta again locally.
                FString Current;
                if (FFileHelper::LoadFileToString(Current, *Path) && Current == EntryId)
                    IFileManager::Get().Delete(*Path);
            }));
    };
    // Bind ClaimSelected to a deliberate player action; do not invoke on startup.
    // A lost response retries the same saved EntryId; never issue a replacement reward.
    // MarkRead/Remove use UserId, EntryId and FOnInboxComplete delegates.
    // Realtime invalidates the cache; HTTP resynchronizes it.
}
