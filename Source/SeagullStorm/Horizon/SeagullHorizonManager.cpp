#include "Horizon/SeagullHorizonManager.h"
#include "SeagullStorm.h"
#include "Core/SeagullGameInstance.h"
#include "Data/SeagullConfigCache.h"
#include "Managers/HorizonLocalizationManager.h"
#include "Managers/HorizonPlayerProfileManager.h"
#include "Managers/HorizonEmailSendingManager.h"

// HorizonSubsystem.h only forward declares its managers: include every manager this facade calls.
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonCloudSaveManager.h"
#include "Managers/HorizonCrashManager.h"
#include "Managers/HorizonFeedbackManager.h"
#include "Managers/HorizonGiftCodeManager.h"
#include "Managers/HorizonLeaderboardManager.h"
#include "Managers/HorizonNewsManager.h"
#include "Managers/HorizonRemoteConfigManager.h"
#include "Managers/HorizonUserLogManager.h"

#if HORIZON_WITH_VALIDATED_ACTIONS
#include "Managers/HorizonValidatedActionsManager.h"
#endif

void USeagullHorizonManager::Initialize(UHorizonSubsystem* InSubsystem)
{
	Subsystem = InSubsystem;
}

void USeagullHorizonManager::ConnectToServer()
{
	if (Subsystem)
	{
		Subsystem->ConnectToServer();
	}
}

bool USeagullHorizonManager::IsConnected() const
{
	return Subsystem && Subsystem->IsConnected();
}

// --- Auth ---

void USeagullHorizonManager::SignUpAnonymous(const FString& Name, TFunction<void(bool)> Callback)
{
	if (!Subsystem || !Subsystem->Auth) { if (Callback) Callback(false); return; }

	Subsystem->Auth->SignUpAnonymous(Name, FOnAuthComplete::CreateLambda(
		[Callback](bool bSuccess)
		{
			if (Callback) Callback(bSuccess);
		}));
}

void USeagullHorizonManager::SignInEmail(const FString& Email, const FString& Password, TFunction<void(bool)> Callback)
{
	if (!Subsystem || !Subsystem->Auth) { if (Callback) Callback(false); return; }

	Subsystem->Auth->SignInEmail(Email, Password, FOnAuthComplete::CreateLambda(
		[Callback](bool bSuccess)
		{
			if (Callback) Callback(bSuccess);
		}));
}

void USeagullHorizonManager::SignUpEmail(const FString& Email, const FString& Password, const FString& Name, TFunction<void(bool)> Callback)
{
	if (!Subsystem || !Subsystem->Auth) { if (Callback) Callback(false); return; }

	Subsystem->Auth->SignUpEmail(Email, Password, Name, FOnAuthComplete::CreateLambda(
		[Callback](bool bSuccess)
		{
			if (Callback) Callback(bSuccess);
		}));
}

void USeagullHorizonManager::RestoreSession(TFunction<void(bool)> Callback)
{
	if (!Subsystem || !Subsystem->Auth) { if (Callback) Callback(false); return; }

	Subsystem->Auth->RestoreSession(FOnAuthComplete::CreateLambda(
		[Callback](bool bSuccess)
		{
			if (Callback) Callback(bSuccess);
		}));
}

void USeagullHorizonManager::SignOut()
{
	if (Subsystem && Subsystem->Auth)
	{
		Subsystem->Auth->SignOut();
	}
}

bool USeagullHorizonManager::IsSignedIn() const
{
	return Subsystem && Subsystem->Auth && Subsystem->Auth->IsSignedIn();
}

FString USeagullHorizonManager::GetDisplayName() const
{
	if (Subsystem && Subsystem->Auth)
	{
		return Subsystem->Auth->GetCurrentUser().DisplayName;
	}
	return TEXT("");
}

FString USeagullHorizonManager::GetUserId() const
{
	if (Subsystem && Subsystem->Auth)
	{
		return Subsystem->Auth->GetCurrentUser().UserId;
	}
	return TEXT("");
}

// --- Remote Config ---

void USeagullHorizonManager::LoadAllConfigs(TFunction<void(bool, const TMap<FString, FString>&)> Callback)
{
	if (!Subsystem || !Subsystem->RemoteConfig) { if (Callback) Callback(false, {}); return; }

	Subsystem->RemoteConfig->GetAllConfigs(false, FOnAllConfigsComplete::CreateLambda(
		[Callback](bool bSuccess, const TMap<FString, FString>& Configs)
		{
			if (Callback) Callback(bSuccess, Configs);
		}));
}

// --- Cloud Save ---

void USeagullHorizonManager::SaveData(const FString& JsonData, TFunction<void(bool)> Callback)
{
	if (!Subsystem || !Subsystem->CloudSave) { if (Callback) Callback(false); return; }

	Subsystem->CloudSave->Save(JsonData, FOnRequestComplete::CreateLambda(
		[Callback](bool bSuccess, const FString& /*Error*/)
		{
			if (Callback) Callback(bSuccess);
		}));
}

void USeagullHorizonManager::LoadData(TFunction<void(bool, const FString&)> Callback)
{
	if (!Subsystem || !Subsystem->CloudSave) { if (Callback) Callback(false, TEXT("")); return; }

	Subsystem->CloudSave->Load(FOnStringComplete::CreateLambda(
		[Callback](bool bSuccess, const FString& Data)
		{
			if (Callback) Callback(bSuccess, Data);
		}));
}

// --- Leaderboard ---

void USeagullHorizonManager::SubmitScore(int64 Score, TFunction<void(bool)> Callback)
{
	if (!Subsystem || !Subsystem->Leaderboard) { if (Callback) Callback(false); return; }

	Subsystem->Leaderboard->SubmitScore(Score, FOnRequestComplete::CreateLambda(
		[Callback](bool bSuccess, const FString& /*Error*/)
		{
			if (Callback) Callback(bSuccess);
		}), FString(), GetLeaderboardKey());
}

void USeagullHorizonManager::GetTop(int32 Limit, TFunction<void(bool, const TArray<FHorizonLeaderboardEntry>&)> Callback)
{
	if (!Subsystem || !Subsystem->Leaderboard) { if (Callback) Callback(false, {}); return; }

	Subsystem->Leaderboard->GetTop(Limit, false, FOnLeaderboardEntriesComplete::CreateLambda(
		[Callback](bool bSuccess, const TArray<FHorizonLeaderboardEntry>& Entries)
		{
			if (Callback) Callback(bSuccess, Entries);
		}), GetLeaderboardKey());
}

void USeagullHorizonManager::GetRank(TFunction<void(bool, const FHorizonLeaderboardEntry&)> Callback)
{
	if (!Subsystem || !Subsystem->Leaderboard) { if (Callback) Callback(false, {}); return; }

	Subsystem->Leaderboard->GetRank(false, FOnLeaderboardRankComplete::CreateLambda(
		[Callback](bool bSuccess, const FHorizonLeaderboardEntry& Entry)
		{
			if (Callback) Callback(bSuccess, Entry);
		}), GetLeaderboardKey());
}

// --- News ---

void USeagullHorizonManager::LoadNews(int32 Limit, const FString& Lang, TFunction<void(bool, const TArray<FHorizonNewsEntry>&)> Callback)
{
	if (!Subsystem || !Subsystem->News) { if (Callback) Callback(false, {}); return; }

	Subsystem->News->LoadNews(Limit, Lang, true, FOnNewsComplete::CreateLambda(
		[Callback](bool bSuccess, const TArray<FHorizonNewsEntry>& Entries)
		{
			if (Callback) Callback(bSuccess, Entries);
		}));
}

// --- Gift Codes ---

void USeagullHorizonManager::ValidateGiftCode(const FString& Code, TFunction<void(bool, bool)> Callback)
{
	if (!Subsystem || !Subsystem->GiftCodes) { if (Callback) Callback(false, false); return; }

	Subsystem->GiftCodes->Validate(Code, FOnGiftCodeValidateComplete::CreateLambda(
		[Callback](bool bRequestSuccess, bool bValid)
		{
			if (Callback) Callback(bRequestSuccess, bValid);
		}));
}

void USeagullHorizonManager::RedeemGiftCode(const FString& Code, TFunction<void(bool, const FString&, const FString&)> Callback)
{
	if (!Subsystem || !Subsystem->GiftCodes) { if (Callback) Callback(false, TEXT(""), TEXT("")); return; }

	Subsystem->GiftCodes->Redeem(Code, FOnGiftCodeRedeemComplete::CreateLambda(
		[Callback](bool bSuccess, const FString& GiftData, const FString& Message)
		{
			if (Callback) Callback(bSuccess, GiftData, Message);
		}));
}

// --- Feedback ---

void USeagullHorizonManager::SubmitFeedback(const FString& Title, const FString& Message, const FString& Category, TFunction<void(bool)> Callback)
{
	if (!Subsystem || !Subsystem->Feedback) { if (Callback) Callback(false); return; }

	Subsystem->Feedback->Submit(Title, Category, Message, TEXT(""), true, FOnRequestComplete::CreateLambda(
		[Callback](bool bSuccess, const FString& /*Error*/)
		{
			if (Callback) Callback(bSuccess);
		}));
}

// --- User Logs ---

void USeagullHorizonManager::LogInfo(const FString& Message)
{
	if (const USeagullGameInstance* GI = GetTypedOuter<USeagullGameInstance>()) { if (GI->bPracticeMode) return; }
	if (!Subsystem || !Subsystem->UserLogs) return;

	Subsystem->UserLogs->Info(Message, FOnUserLogComplete::CreateLambda(
		[](bool bSuccess, const FString& /*LogId*/, const FString& /*CreatedAt*/)
		{
			UE_LOG(LogSeagullStorm, Log, TEXT("User log submitted: %s"), bSuccess ? TEXT("OK") : TEXT("Failed"));
		}));
}

void USeagullHorizonManager::LogWarn(const FString& Message)
{
	if (const USeagullGameInstance* GI = GetTypedOuter<USeagullGameInstance>()) { if (GI->bPracticeMode) return; }
	if (!Subsystem || !Subsystem->UserLogs) return;

	Subsystem->UserLogs->Warn(Message, FOnUserLogComplete::CreateLambda(
		[](bool bSuccess, const FString& /*LogId*/, const FString& /*CreatedAt*/)
		{
			UE_LOG(LogSeagullStorm, Log, TEXT("User warn log submitted: %s"), bSuccess ? TEXT("OK") : TEXT("Failed"));
		}));
}

// --- Crash Reporting ---

void USeagullHorizonManager::StartCrashCapture()
{
	if (Subsystem && Subsystem->Crashes)
	{
		Subsystem->Crashes->StartCapture();
	}
}

void USeagullHorizonManager::RecordBreadcrumb(const FString& Type, const FString& Message)
{
	if (Subsystem && Subsystem->Crashes)
	{
		Subsystem->Crashes->RecordBreadcrumb(Type, Message);
	}
}

void USeagullHorizonManager::SetCrashCustomKey(const FString& Key, const FString& Value)
{
	if (Subsystem && Subsystem->Crashes)
	{
		Subsystem->Crashes->SetCustomKey(Key, Value);
	}
}

void USeagullHorizonManager::RecordException(const FString& Error, const FString& StackTrace)
{
	if (const USeagullGameInstance* GI = GetTypedOuter<USeagullGameInstance>()) { if (GI->bPracticeMode) return; }
	if (Subsystem && Subsystem->Crashes)
	{
		Subsystem->Crashes->RecordException(Error, StackTrace);
	}
}

// --- Validated Actions ---

bool USeagullHorizonManager::IsValidatedActionsCompiledIn()
{
#if HORIZON_WITH_VALIDATED_ACTIONS
	return true;
#else
	return false;
#endif
}

void USeagullHorizonManager::StartValidatedRun(const FString& LeaderboardKey, TFunction<void(bool, int32, const FString&)> Callback)
{
#if HORIZON_WITH_VALIDATED_ACTIONS
	if (!Subsystem || !Subsystem->ValidatedActions) { if (Callback) Callback(false, 0, TEXT("NOT_SUPPORTED")); return; }

	UHorizonValidatedActionsManager* Validated = Subsystem->ValidatedActions;
	EvidenceStatus = TEXT("No evidence requested");

	// Upload the input log right away when the server asks for evidence (top N, flagged runs).
	Validated->bAutoUploadEvidence = true;
	Validated->OnEvidenceUploaded.AddUniqueDynamic(this, &USeagullHorizonManager::HandleEvidenceUploaded);
	Validated->OnEvidenceUploadFailed.AddUniqueDynamic(this, &USeagullHorizonManager::HandleEvidenceUploadFailed);

	Validated->StartRun(LeaderboardKey, FOnValidatedRunStarted::CreateLambda(
		[Callback](bool bSuccess, const FHorizonValidatedRun& Run, const FString& ErrorCode, const FString& /*ErrorMessage*/)
		{
			if (Callback) Callback(bSuccess, Run.Seed, ErrorCode);
		}));
#else
	if (Callback) Callback(false, 0, TEXT("NOT_SUPPORTED"));
#endif
}

bool USeagullHorizonManager::HasValidatedRun() const
{
#if HORIZON_WITH_VALIDATED_ACTIONS
	return Subsystem && Subsystem->ValidatedActions && Subsystem->ValidatedActions->HasActiveRun();
#else
	return false;
#endif
}

void USeagullHorizonManager::DiscardValidatedRun()
{
#if HORIZON_WITH_VALIDATED_ACTIONS
	if (Subsystem && Subsystem->ValidatedActions)
	{
		Subsystem->ValidatedActions->DiscardRun();
	}
#endif
}

void USeagullHorizonManager::SubmitValidatedScore(int64 Score, const TArray<uint8>& InputLog, const FString& Stage,
	const FString& CoinsKey, int64 Coins, TFunction<void(bool, int64, const FString&)> Callback)
{
#if HORIZON_WITH_VALIDATED_ACTIONS
	if (!Subsystem || !Subsystem->ValidatedActions) { if (Callback) Callback(false, 0, TEXT("NOT_SUPPORTED")); return; }

	TArray<FHorizonEarnedValue> Earned;
	if (!CoinsKey.IsEmpty() && Coins > 0)
	{
		Earned.Add(FHorizonEarnedValue(CoinsKey, Coins));
	}

	// Empty LeaderboardKey: the board the ticket was bound to at StartValidatedRun.
	Subsystem->ValidatedActions->SubmitValidated(Score, InputLog, Stage, FString(), Earned,
		FOnValidatedSubmitComplete::CreateLambda(
			[Callback](bool bSuccess, const FHorizonValidatedSubmitResult& Result, const FString& ErrorCode, const FString& /*ErrorMessage*/)
			{
				if (Callback) Callback(bSuccess && Result.bAccepted, Result.Rank, bSuccess && !Result.bAccepted ? FString(TEXT("SERVER_RESULT_NOT_ACCEPTED")) : ErrorCode);
			}));
#else
	if (Callback) Callback(false, 0, TEXT("NOT_SUPPORTED"));
#endif
}

FString USeagullHorizonManager::GetLastSubmitErrorCode() const
{
#if HORIZON_WITH_VALIDATED_ACTIONS
	if (Subsystem && Subsystem->Leaderboard)
	{
		return Subsystem->Leaderboard->GetLastSubmitErrorCode();
	}
#endif
	return FString();
}

FString USeagullHorizonManager::DescribeScoreError(const FString& ErrorCode)
{
	if (ErrorCode == TEXT("DURATION_TOO_SHORT")) return TEXT("Run too short to rank.");
	if (ErrorCode == TEXT("SCORE_ABOVE_MAX") || ErrorCode == TEXT("STAGE_SCORE_ABOVE_MAX")
		|| ErrorCode == TEXT("SCORE_RATE_TOO_HIGH")) return TEXT("Score looks off, not ranked.");
	if (ErrorCode == TEXT("SCORE_BELOW_MIN") || ErrorCode == TEXT("STAGE_SCORE_BELOW_MIN")) return TEXT("Score too low to rank.");
	if (ErrorCode == TEXT("STAGE_REQUIRED") || ErrorCode == TEXT("STAGE_UNKNOWN")) return TEXT("This wave is not ranked.");
	if (ErrorCode == TEXT("TICKET_EXPIRED")) return TEXT("Run ticket expired.");
	if (ErrorCode.StartsWith(TEXT("TICKET_")) || ErrorCode == TEXT("LEADERBOARD_MISMATCH")) return TEXT("Run ticket not valid.");
	if (ErrorCode == TEXT("RUN_RATE_LIMITED") || ErrorCode == TEXT("RUN_CAPACITY_REACHED")) return TEXT("Too many runs, try later.");
	if (ErrorCode == TEXT("SCORE_LIMIT_REACHED")) return TEXT("Score limit reached.");
	if (ErrorCode == TEXT("PLAYER_BANNED")) return TEXT("You cannot rank here.");
	if (ErrorCode == TEXT("VALIDATED_SUBMIT_REQUIRED")) return TEXT("Board takes validated runs only.");
	if (ErrorCode == TEXT("UNKNOWN_VALUE_KEY") || ErrorCode == TEXT("DUPLICATE_VALUE_KEY")
		|| ErrorCode.StartsWith(TEXT("EARNED_")) || ErrorCode == TEXT("INSUFFICIENT_BALANCE")) return TEXT("Coin reward rejected.");
	if (ErrorCode == TEXT("SESSION_REQUIRED")) return TEXT("Sign in to rank.");
	if (ErrorCode == TEXT("NOT_SUPPORTED")) return TEXT("Server has no validated runs.");
	if (ErrorCode == TEXT("CONNECTION_FAILED")) return TEXT("Offline, score not sent.");
	return ErrorCode.IsEmpty() ? FString(TEXT("Score not sent.")) : FString::Printf(TEXT("Score not sent (%s)."), *ErrorCode);
}

void USeagullHorizonManager::HandleEvidenceUploaded(const FString& RunId, int32 Bytes)
{
	EvidenceStatus = FString::Printf(TEXT("Evidence uploaded / %d bytes"), Bytes);
	UE_LOG(LogSeagullStorm, Log, TEXT("Validated run %s: input log uploaded as evidence (%d bytes)"), *RunId, Bytes);
}

void USeagullHorizonManager::HandleEvidenceUploadFailed(const FString& RunId, const FString& ErrorCode, const FString& ErrorMessage)
{
	EvidenceStatus = FString::Printf(TEXT("Score accepted / evidence upload failed: %s"), *ErrorCode);
	UE_LOG(LogSeagullStorm, Warning, TEXT("Validated run %s: evidence upload failed (%s): %s"), *RunId, *ErrorCode, *ErrorMessage);
}

FString USeagullHorizonManager::GetLeaderboardKey() const
{
    USeagullGameInstance* GI = GetTypedOuter<USeagullGameInstance>();
    return GI ? GI->GetConfigCache()->ValidatedRunsBoard : FString(TEXT("default"));
}

void USeagullHorizonManager::ListBoards(TFunction<void(bool, const TArray<FHorizonLeaderboardBoard>&)> Callback)
{
    if (!Subsystem || !Subsystem->Leaderboard) { if (Callback) Callback(false, {}); return; }
    Subsystem->Leaderboard->ListBoards(FOnLeaderboardBoardsComplete::CreateLambda(
        [Callback](bool bSuccess, const TArray<FHorizonLeaderboardBoard>& Boards) { if (Callback) Callback(bSuccess, Boards); }));
}

void USeagullHorizonManager::GetAround(int32 Range, TFunction<void(bool, const TArray<FHorizonLeaderboardEntry>&)> Callback)
{
    if (!Subsystem || !Subsystem->Leaderboard) { if (Callback) Callback(false, {}); return; }
    Subsystem->Leaderboard->GetAround(Range, false, FOnLeaderboardEntriesComplete::CreateLambda(
        [Callback](bool bSuccess, const TArray<FHorizonLeaderboardEntry>& Entries) { if (Callback) Callback(bSuccess, Entries); }), GetLeaderboardKey());
}

void USeagullHorizonManager::GetValidatedState(TFunction<void(bool, const FHorizonPlayerState&, const FString&)> Callback)
{
    if (!Subsystem || !Subsystem->ValidatedActions) { if (Callback) Callback(false, {}, TEXT("NOT_SUPPORTED")); return; }
    Subsystem->ValidatedActions->GetState(FOnPlayerStateLoaded::CreateLambda(
        [Callback](bool bSuccess, const FHorizonPlayerState& State, const FString& Code, const FString&) { if (Callback) Callback(bSuccess, State, Code); }));
}

void USeagullHorizonManager::LoadLocalization(const FString& Lang, TFunction<void(bool, const TMap<FString, FString>&)> Callback)
{
    if (!Subsystem || !Subsystem->Localization) { if (Callback) Callback(false, {}); return; }
    Subsystem->Localization->SetLanguage(Lang);
    Subsystem->Localization->GetAllLocalizations(Lang, FOnAllLocalizationsComplete::CreateLambda(
        [Callback](bool bSuccess, const TMap<FString, FString>& Values) { if (Callback) Callback(bSuccess, Values); }));
}

void USeagullHorizonManager::GetPlayerProfile(TFunction<void(bool, const FHorizonPlayerProfileResult&, const FString&)> Callback)
{
    if (!Subsystem || !Subsystem->PlayerProfile) { if (Callback) Callback(false, {}, TEXT("NOT_SUPPORTED")); return; }
    Subsystem->PlayerProfile->GetProfile(FOnPlayerProfileComplete::CreateLambda(
        [Callback](bool Ok, const FHorizonPlayerProfileResult& Result, const FString& Code, const FString&) { if (Callback) Callback(Ok, Result, Code); }));
}

void USeagullHorizonManager::SetPlayerAvatar(const FString& AvatarId, TFunction<void(bool, const FHorizonPlayerProfileResult&, const FString&)> Callback)
{
    if (!Subsystem || !Subsystem->PlayerProfile || !Subsystem->PlayerProfile->HasCurrentProfile())
    { if (Callback) Callback(false, {}, TEXT("LOAD_PROFILE_FIRST")); return; }
    const FHorizonPlayerProfileResult& Current = Subsystem->PlayerProfile->GetCurrentProfile();
    if (!Current.IsAvailable(AvatarId)) { if (Callback) Callback(false, Current, TEXT("COSMETIC_NOT_AVAILABLE")); return; }
    // Preserve the other slots: SetProfile replaces the whole profile.
    Subsystem->PlayerProfile->SetProfile(AvatarId, Current.Profile.FrameId, Current.Profile.Badges,
        FOnPlayerProfileComplete::CreateLambda(
            [Callback](bool Ok, const FHorizonPlayerProfileResult& Result, const FString& Code, const FString&) { if (Callback) Callback(Ok, Result, Code); }));
}

void USeagullHorizonManager::SendOwnAccountEmail(const FString& TemplateSlug, TFunction<void(bool, const FString&)> Callback)
{
    if (!Subsystem || !Subsystem->EmailSending || !IsSignedIn()) { if (Callback) Callback(false, TEXT("Sign in with an email account first.")); return; }
    if (!Subsystem->Auth || Subsystem->Auth->GetCurrentUser().Email.IsEmpty())
    { if (Callback) Callback(false, TEXT("This player has no email address. Use an email account for the email demo.")); return; }
    // Called only by the explicit panel button, never automatically after a run.
    Subsystem->EmailSending->SendEmail(GetUserId(), TemplateSlug, {}, TEXT("en"), FOnSendEmailComplete::CreateLambda(
        [Callback](bool Ok, const FSendEmailResponse& Response)
        {
            if (Callback) Callback(Ok, Ok ? FString::Printf(TEXT("Email request accepted / %s / id %s"), *Response.Status, *Response.Id)
                : FString(TEXT("Email request failed / check template and the account email.")));
        }));
}
