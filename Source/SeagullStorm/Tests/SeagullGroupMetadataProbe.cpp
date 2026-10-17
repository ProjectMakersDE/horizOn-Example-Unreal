#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Examples/HorizonGroupsExample.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSeagullGroupMetadataProbe,"Seagull.Acceptance.GroupMetadataNulProbe",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FSeagullGroupMetadataProbe::RunTest(const FString&)
{
    const TCHAR Chars[]={TCHAR('a'),TCHAR(0),TCHAR(0x96ea)};
    const FString Original(UE_ARRAY_COUNT(Chars),Chars);
    TestEqual(TEXT("Native input has all three code units"),Original.Len(),3);
    auto* Pending=Cast<UHorizonGroupPendingAction>(UGameplayStatics::CreateSaveGameObject(UHorizonGroupPendingAction::StaticClass()));
    Pending->Action=TEXT("update");Pending->ExpectedRevision=1;Pending->Metadata.Name=Original;Pending->Metadata.Description=Original;
    TArray<uint8> Bytes;TestTrue(TEXT("Official Groups original serializes"),UGameplayStatics::SaveGameToMemory(Pending,Bytes));
    auto* Restored=Cast<UHorizonGroupPendingAction>(UGameplayStatics::LoadGameFromMemory(Bytes));
    const bool Exact=Restored&&Restored->Metadata.Name==Original&&Restored->Metadata.Description==Original;
    FString Output; if(FParse::Value(FCommandLine::Get(),TEXT("SeagullProbeOutput="),Output))
        FFileHelper::SaveStringToFile(*FString::Printf(TEXT("{\"inputUnits\":%d,\"restoredNameUnits\":%d,\"restoredDescriptionUnits\":%d,\"exact\":%s}\n"),Original.Len(),Restored?Restored->Metadata.Name.Len():-1,Restored?Restored->Metadata.Description.Len():-1,Exact?TEXT("true"):TEXT("false")),*Output);
    TestTrue(TEXT("NUL and Unicode metadata survive the actual official SaveGame original roundtrip"),Exact);
    return true;
}
#endif
