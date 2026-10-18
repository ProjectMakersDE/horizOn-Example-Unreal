#include "HorizonSubsystem.h"
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonGroupsManager.h"
#include "Models/HorizonGroups.h"

// Call from an explicit action after restoring the existing matching session.
void ReadMyGroups(UHorizonSubsystem* SDK)
{
    if (!SDK || !SDK->Auth || !SDK->Auth->IsSignedIn() || !SDK->Groups) return;
    const FString UserId = SDK->Auth->GetCurrentUser().UserId;
    SDK->Groups->GetGroups(UserId,
        FOnGroupsComplete::CreateLambda([](bool bSuccess,
            const FHorizonMyGroupsView& Result,
            const FString& Code, const FString& Message)
        {
            if (!bSuccess) return;
            for (const FHorizonGroupView& Group : Result.Groups)
                UE_LOG(LogTemp, Log, TEXT("Group %s: revision %lld"), *Group.Id, Group.Revision);
            // Result.Limits is current policy, not a cap on retained reads.
            // GetMembers(User,Group,Callback,TEXT(""),1) reads one explicit page.
            // Keep its revision/cursor; restart on revision change. Unknown dates stay optional.
        }));
}
// Durable mutations: SDK Examples/HorizonGroupsExample.h and its journal.
// Keep original project/player, body, revision and pending requestId before one send.
// An unknown create is reconciled; never replace the original player revision.
