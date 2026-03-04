#include "Horizon/SeagullHorizonManager.h"
#include "SeagullStorm.h"

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
		}));
}

void USeagullHorizonManager::GetTop(int32 Limit, TFunction<void(bool, const TArray<FHorizonLeaderboardEntry>&)> Callback)
{
	if (!Subsystem || !Subsystem->Leaderboard) { if (Callback) Callback(false, {}); return; }

	Subsystem->Leaderboard->GetTop(Limit, false, FOnLeaderboardEntriesComplete::CreateLambda(
		[Callback](bool bSuccess, const TArray<FHorizonLeaderboardEntry>& Entries)
		{
			if (Callback) Callback(bSuccess, Entries);
		}));
}

void USeagullHorizonManager::GetRank(TFunction<void(bool, const FHorizonLeaderboardEntry&)> Callback)
{
	if (!Subsystem || !Subsystem->Leaderboard) { if (Callback) Callback(false, {}); return; }

	Subsystem->Leaderboard->GetRank(false, FOnLeaderboardRankComplete::CreateLambda(
		[Callback](bool bSuccess, const FHorizonLeaderboardEntry& Entry)
		{
			if (Callback) Callback(bSuccess, Entry);
		}));
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
	if (!Subsystem || !Subsystem->UserLogs) return;

	Subsystem->UserLogs->Info(Message, FOnUserLogComplete::CreateLambda(
		[](bool bSuccess, const FString& /*LogId*/, const FString& /*CreatedAt*/)
		{
			UE_LOG(LogSeagullStorm, Log, TEXT("User log submitted: %s"), bSuccess ? TEXT("OK") : TEXT("Failed"));
		}));
}

void USeagullHorizonManager::LogWarn(const FString& Message)
{
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
	if (Subsystem && Subsystem->Crashes)
	{
		Subsystem->Crashes->RecordException(Error, StackTrace);
	}
}
