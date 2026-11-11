#include "HorizonSubsystem.h"
#include "Engine/GameInstance.h"
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonPlayerValuesManager.h"

// Existing SDK/project configuration and authenticated player session required.
// Call this function explicitly after reviewing the example and its data.
void RunReviewedPlayerValuesDemo(UGameInstance* GameInstance)
{
    if (!GameInstance) return;
    auto* Existing = GameInstance->GetSubsystem<UHorizonSubsystem>();
    if (!Existing || !Existing->Auth || !Existing->Auth->IsSignedIn()) return;
    const auto GetGameInstance = [GameInstance]() { return GameInstance; };
    UHorizonSubsystem* SDK = GetGameInstance()->GetSubsystem<UHorizonSubsystem>();
    const FString UserId = SDK->Auth->GetCurrentUser().UserId;
    SDK->PlayerValues->GetState(UserId, FOnPlayerValuesComplete::CreateLambda(
        [](bool bSuccess, const FHorizonPlayerValuesState& State,
            const FString& ErrorCode, const FString& ErrorMessage)
        {
            if (!bSuccess) { UE_LOG(LogTemp, Warning, TEXT("Values: %s"), *ErrorCode); return; }
            for (const auto& Value : State.Values)
                UE_LOG(LogTemp, Log, TEXT("%s=%lld"), *Value.Key, Value.Balance);
        }));
    // This minimal first call never starts a purchase or creates an operation ID.
    // Use the official game/SDK durable purchase flow for writes: preserve original
    // user/offer/operationId before one send and retain it after uncertain outcomes.
    // Read current inventory separately; never reconstruct old item references.
}
