#include "HorizonSubsystem.h"
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonMatchesManager.h"
#include "Models/HorizonMatches.h"

// Verified SDK Develop API. Called from an explicit player action.
void ReadMyMatches(UHorizonSubsystem* SDK)
{
    if (!SDK || !SDK->Auth || !SDK->Matches) return;
    const FString UserId = SDK->Auth->GetCurrentUser().UserId;
    SDK->Matches->GetMatches(UserId,
        FOnMatchesComplete::CreateLambda([](bool bSuccess,
            const FHorizonMyMatches& Result,
            const FString& Code, const FString& Message)
        {
            if (!bSuccess) return;
            for (const FHorizonMatchSummary& Match : Result.Recent)
                UE_LOG(LogTemp, Log, TEXT("Match %s: %s"), *Match.Id, *Match.Status);
            // Recent may include an active match after this player's forfeit.
            // Respect Result.PollAfterSeconds. No automatic poll or timeout action.
        }));
}
// Executable two-player recipe: Examples/HorizonMatchesExample.h, AHorizonMatchesExample.
// Explicit Refresh/CreateLobby/WatchMatch/StartAction and RetryOriginal/reconciliation.
// The project+player journal preserves original bytes/revisions across regional host changes.
// JoinMatch(User,Id,OriginalRevision,FOnMatchComplete),
// StartMatch(User,Id,OriginalRevision,TurnOrder,FOnMatchComplete),
// SubmitTurn(User,Id,OriginalRevision,Bytes,bFinish,FOnMatchComplete),
// ReportResult(User,Id,OriginalFinalRevision,Bytes,SortedWinners,FOnMatchReportedComplete).
// These are native delegates: BindUObject/CreateLambda, never BindDynamic.
// Unknown create is reconciled, never silently replayed with a fresh ledger revision.
// Confirmed ranked matches use StartRankedRun and the existing Validated Actions pipeline.
