#include "HorizonSubsystem.h"
#include "Engine/GameInstance.h"
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonAchievementsManager.h"
#include "Models/HorizonAchievements.h"

// Existing SDK/project configuration and authenticated player session required.
// Call this function explicitly after reviewing the example and its data.
// This explicit action unlocks the shown configured achievement key.
void RunReviewedAchievementsDemo(UGameInstance* GameInstance)
{
    if (!GameInstance) return;
    auto* Existing = GameInstance->GetSubsystem<UHorizonSubsystem>();
    if (!Existing || !Existing->Auth || !Existing->Auth->IsSignedIn()) return;
    const auto GetGameInstance = [GameInstance]() { return GameInstance; };
    UHorizonSubsystem* SDK = GetGameInstance()->GetSubsystem<UHorizonSubsystem>();
    const FString UserId = SDK->Auth->GetCurrentUser().UserId;
    SDK->Achievements->GetAchievements(UserId, FOnAchievementsComplete::CreateLambda(
        [](bool bSuccess, const FHorizonAchievementsSnapshot& Snapshot,
            const FString& Code, const FString& Message)
        {
            if (!bSuccess) { UE_LOG(LogTemp, Warning, TEXT("Achievements: %s"), *Code); return; }
            for (const auto& Progress : Snapshot.Progress)
                UE_LOG(LogTemp, Log, TEXT("%s: %s"), *Progress.Key, *Progress.RewardState);
        }));
    // On a matching game action, unlock configured manual first_steps. Retry this same key.
    SDK->Achievements->Unlock(UserId, TEXT("first_steps"), FOnAchievementUnlockComplete::CreateLambda(
        [](bool bSuccess, const FHorizonAchievementUnlockResponse& Result,
            const FString& Code, const FString& Message)
        {
            if (!bSuccess) { UE_LOG(LogTemp, Warning, TEXT("Keep original key: %s"), *Code); return; }
            // Complete progress means rewards already committed with the unlock.
            // Refresh PlayerValues explicitly when displaying the current balance.
            UE_LOG(LogTemp, Log, TEXT("New unlocks: %d"), Result.NewlyUnlocked.Num());
        }));
    // No Inbox claim or local value credit. A lost reply is retried with the original key.
}
