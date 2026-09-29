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

	// --- Validated Actions ---
	// Compiled only with HORIZON_WITH_VALIDATED_ACTIONS=1 (SeagullStorm.Build.cs). Without it
	// every call reports "NOT_SUPPORTED" and the game keeps the normal SubmitScore path.
	static bool IsValidatedActionsCompiledIn();

	/** Asks the server for a run ticket bound to LeaderboardKey. Callback(bSuccess, Seed, ErrorCode). */
	void StartValidatedRun(const FString& LeaderboardKey, TFunction<void(bool, int32, const FString&)> Callback);

	/** True while a started ticket waits for its submit. */
	bool HasValidatedRun() const;

	/** Drops the current ticket without submitting it. */
	void DiscardValidatedRun();

	/**
	 * Submits the run with its input log. CoinsKey empty sends no earned values; otherwise
	 * Coins is sent as earned value CoinsKey (the rules of the API key must define it).
	 * Callback(bSuccess, Rank, ErrorCode); Rank is 0 when unknown.
	 */
	void SubmitValidatedScore(int64 Score, const TArray<uint8>& InputLog, const FString& Stage,
		const FString& CoinsKey, int64 Coins, TFunction<void(bool, int64, const FString&)> Callback);

	/** Error code of the last failed SubmitScore (for example VALIDATED_SUBMIT_REQUIRED), empty if unknown. */
	FString GetLastSubmitErrorCode() const;

	/** Player facing text for a Validated Actions or leaderboard error code. */
	static FString DescribeScoreError(const FString& ErrorCode);

private:
	UPROPERTY()
	UHorizonSubsystem* Subsystem = nullptr;

	UFUNCTION()
	void HandleEvidenceUploaded(const FString& RunId, int32 Bytes);

	UFUNCTION()
	void HandleEvidenceUploadFailed(const FString& RunId, const FString& ErrorCode, const FString& ErrorMessage);
};
