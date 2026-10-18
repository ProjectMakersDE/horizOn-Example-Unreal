#include "HorizonSubsystem.h"
#include "Engine/GameInstance.h"
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonStatisticsManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

// Existing SDK/project configuration and authenticated player session required.
// Call this function explicitly after reviewing the example and its data.
// This explicit action reports the shown persisted batch.
void RunReviewedStatisticsDemo(UGameInstance* GameInstance)
{
    if (!GameInstance) return;
    auto* Existing = GameInstance->GetSubsystem<UHorizonSubsystem>();
    if (!Existing || !Existing->Auth || !Existing->Auth->IsSignedIn()) return;
    const auto GetGameInstance = [GameInstance]() { return GameInstance; };
    UHorizonSubsystem* SDK = GetGameInstance()->GetSubsystem<UHorizonSubsystem>();
    SDK->Statistics->GetStatistics(FOnStatisticsLoaded::CreateLambda(
        [SDK](bool bSuccess, const FHorizonStatisticsSnapshot& Snapshot,
            const FString& Code, const FString& Message)
        {
            if (!bSuccess) { UE_LOG(LogTemp, Warning, TEXT("Statistics: %s"), *Code); return; }
            const FString Path = FPaths::Combine(FPaths::ProjectSavedDir(),
                TEXT("statistics_") + Snapshot.UserId + TEXT(".json"));
            FString Json;
            if (IFileManager::Get().FileExists(*Path)) {
                if (!FFileHelper::LoadFileToString(Json, *Path)) return;
            } else {
                const FString Id = SDK->Statistics->CreateOperationId();
                Json = FString::Printf(TEXT("{\"operationId\":\"%s\",\"kills\":3,\"best_score\":250}"), *Id);
                if (!FFileHelper::SaveStringToFile(Json, *Path)) return;
            }
            TSharedPtr<FJsonObject> Pending;
            if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Pending) || !Pending.IsValid()) return;
            TArray<FHorizonStatisticUpdate> Updates;
            for (const FString& Key : {FString(TEXT("kills")), FString(TEXT("best_score"))}) {
                FHorizonStatisticUpdate Update;
                Update.Key = Key;
                double Value = 0;
                if (!Pending->TryGetNumberField(Key, Value) || !FMath::IsFinite(Value)
                    || Value < -9007199254740991.0 || Value > 9007199254740991.0) return;
                const int64 Exact = static_cast<int64>(Value);
                if (static_cast<double>(Exact) != Value) return;
                Update.Value = Exact;
                Updates.Add(Update);
            }
            SDK->Statistics->UpdateStatistics(Updates, Pending->GetStringField(TEXT("operationId")),
                FOnStatisticsLoaded::CreateLambda([Path, Json](bool bOk, const FHorizonStatisticsSnapshot& Current,
                    const FString& ErrorCode, const FString& ErrorMessage)
                {
                    if (bOk) {
                        FString SavedOriginal;
                        if (FFileHelper::LoadFileToString(SavedOriginal, *Path) && SavedOriginal.Equals(Json, ESearchCase::CaseSensitive))
                            IFileManager::Get().Delete(*Path);
                    }
                    else UE_LOG(LogTemp, Warning, TEXT("Keep original statistics batch: %s"), *ErrorCode);
                }));
        }));
}
