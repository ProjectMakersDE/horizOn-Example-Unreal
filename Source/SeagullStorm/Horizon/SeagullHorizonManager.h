#pragma once

#include "CoreMinimal.h"
#include "HorizonSubsystem.h"
#include "Models/HorizonLeaderboardEntry.h"
#include "Models/HorizonNewsEntry.h"
#include "SeagullHorizonManager.generated.h"

UCLASS()
class USeagullHorizonManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UHorizonSubsystem* InSubsystem);

	// --- Connection ---
	void ConnectToServer();
	bool IsConnected() const;

	// --- Auth ---
	void SignUpAnonymous(const FString& Name, TFunction<void(bool)> Callback);
	void SignInEmail(const FString& Email, const FString& Password, TFunction<void(bool)> Callback);
	void SignUpEmail(const FString& Email, const FString& Password, const FString& Name, TFunction<void(bool)> Callback);
	void RestoreSession(TFunction<void(bool)> Callback);
	void SignOut();
	bool IsSignedIn() const;
	FString GetDisplayName() const;
	FString GetUserId() const;

	// --- Remote Config ---
	void LoadAllConfigs(TFunction<void(bool, const TMap<FString, FString>&)> Callback);

	// --- Cloud Save ---
	void SaveData(const FString& JsonData, TFunction<void(bool)> Callback);
	void LoadData(TFunction<void(bool, const FString&)> Callback);

	// --- Leaderboard ---
	void SubmitScore(int64 Score, TFunction<void(bool)> Callback);
	void GetTop(int32 Limit, TFunction<void(bool, const TArray<FHorizonLeaderboardEntry>&)> Callback);
	void GetRank(TFunction<void(bool, const FHorizonLeaderboardEntry&)> Callback);

	// --- News ---
	void LoadNews(int32 Limit, const FString& Lang, TFunction<void(bool, const TArray<FHorizonNewsEntry>&)> Callback);

	// --- Gift Codes ---
	void ValidateGiftCode(const FString& Code, TFunction<void(bool, bool)> Callback);
	void RedeemGiftCode(const FString& Code, TFunction<void(bool, const FString&, const FString&)> Callback);

	// --- Feedback ---
	void SubmitFeedback(const FString& Title, const FString& Message, const FString& Category, TFunction<void(bool)> Callback);

	// --- User Logs ---
	void LogInfo(const FString& Message);
	void LogWarn(const FString& Message);

	// --- Crash Reporting ---
	void StartCrashCapture();
	void RecordBreadcrumb(const FString& Type, const FString& Message);
	void SetCrashCustomKey(const FString& Key, const FString& Value);
	void RecordException(const FString& Error, const FString& StackTrace);

private:
	UPROPERTY()
	UHorizonSubsystem* Subsystem = nullptr;
};
