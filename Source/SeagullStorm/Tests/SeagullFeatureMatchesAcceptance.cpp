#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Core/SeagullGameInstance.h"
#include "Horizon/SeagullHorizonManager.h"
#include "UI/SeagullFeatureLab.h"
#include "Examples/HorizonMatchesExample.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

namespace SeagullFeatureMatches
{
const FString MatchId=TEXT("aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa");
const FString User=TEXT("00000000-0000-4000-8000-000000000001"),Other=TEXT("00000000-0000-4000-8000-000000000002");
class FFlow final:public IAutomationLatentCommand
{
    FAutomationTestBase* Test;double Started=FPlatformTime::Seconds();int32 Step=0;bool AuthDone=false,AuthOk=false;TWeakObjectPtr<USeagullFeatureLab> Lab;
    bool Invoke(const FString& Action,const FString& Arguments)
    {
        FString Error;const bool Accepted=Lab->Execute(TEXT("Matches"),Action,Arguments,Error);
        if(!Accepted)Test->AddError(TEXT("Actual Lab dispatch refused ")+Action+TEXT(": ")+Error);
        return Accepted;
    }
public:
    explicit FFlow(FAutomationTestBase* T):Test(T){}
    bool Update()override
    {
        if(FPlatformTime::Seconds()-Started>40){Test->AddError(FString::Printf(TEXT("Actual Match Lab workflow timed out at step%d"),Step));return true;}
        UWorld* World=nullptr;for(const auto& C:GEngine->GetWorldContexts())if(C.World()&&C.World()->IsGameWorld()){World=C.World();break;}if(!World)return false;
        auto* GI=Cast<USeagullGameInstance>(World->GetGameInstance());if(!GI)return false;auto* H=GI->GetHorizonManager();
        if(!Lab.IsValid()){TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World,Widgets,USeagullFeatureLab::StaticClass(),true);if(Widgets.Num()!=1)return false;Lab=Cast<USeagullFeatureLab>(Widgets[0]);}
        AHorizonMatchesExample* Sample=nullptr;for(TActorIterator<AHorizonMatchesExample> It(World);It;++It)if(IsValid(*It)){Sample=*It;break;}
        const auto Current=Sample?Sample->GetMatch():FHorizonMatchRead{};
        const FString Base=TEXT("{\"id\":\"")+MatchId+TEXT("\",");
        const FString Ids=TEXT("[\"")+User+TEXT("\",\"")+Other+TEXT("\"]");
        switch(Step)
        {
        case 0:H->ConnectToServer();++Step;break;
        case 1:if(!H->IsConnected())return false;H->SignInEmail(TEXT("lab@example.invalid"),TEXT("synthetic-password"),[this](bool Ok){AuthDone=true;AuthOk=Ok;});++Step;break;
        case 2:
            if(!AuthDone)return false;if(!AuthOk){Test->AddError(TEXT("Synthetic existing-player sign-in failed"));return true;}GI->BeginOnlineSession(false);
            if(!Invoke(TEXT("read"),TEXT("{\"id\":\"")+MatchId+TEXT("\"}")))return true;++Step;break;
        case 3:
            if(!Current.bPresent||Current.Match.Revision!=7)return false;
            {FString Error;Test->TestFalse(TEXT("Missing full turn-order array is refused before HTTP"),Lab->Execute(TEXT("Matches"),TEXT("start"),Base+TEXT("\"revision\":\"7\"}"),Error));Test->TestEqual(TEXT("Explicit array error"),Error,FString(TEXT("MATCH_IDS_ARRAY_REQUIRED")));}
            if(!Invoke(TEXT("start"),Base+TEXT("\"revision\":\"7\",\"ids\":")+Ids+TEXT("}")))return true;++Step;break;
        case 4:
            if(!Current.bPresent||Current.Match.Revision!=8)return false;
            Test->TestEqual(TEXT("Actual start retains both turn-order IDs"),Current.Match.TurnOrder.Num(),2);
            if(!Invoke(TEXT("turn"),Base+TEXT("\"revision\":\"8\",\"base64\":\"AAE=\",\"flag\":true}")))return true;++Step;break;
        case 5:
            if(!Current.bPresent||Current.Match.Revision!=9||!Current.Match.bHasFinalStateRevision)return false;
            if(!Invoke(TEXT("result"),Base+TEXT("\"revision\":\"9\",\"base64\":\"AAE=\",\"ids\":")+Ids+TEXT("}")))return true;++Step;break;
        case 6:
            if(!Sample||Sample->LastErrorCode!=TEXT("TEMPORARY"))return false;
            Lab->RemoveFromParent();Lab=CreateWidget<USeagullFeatureLab>(World->GetFirstPlayerController());Lab->AddToViewport(100);
            if(!Invoke(TEXT("retry-original"),TEXT("{}")))return true;++Step;break;
        case 7:
            if(!Current.bPresent||Current.Match.Revision!=10)return false;
            Test->TestEqual(TEXT("Original result has one authoritative report"),Current.Match.Reports.Num(),1);
            if(Current.Match.Reports.Num()==1){Test->TestEqual(TEXT("Both explicit winner IDs survive stored original/recreated Lab/retry"),Current.Match.Reports[0].WinnerIds.Num(),2);Test->TestTrue(TEXT("Result bytes remain unchanged"),Current.Match.Reports[0].Result==TArray<uint8>({0,1}));}
            Test->AddInfo(TEXT("Actual Lab start/finish/result dispatch used complete ID arrays; uncertain result survived screen recreation and retried the original HTTP bytes."));return true;
        }
        return false;
    }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSeagullFeatureMatches,"Seagull.Acceptance.CurrentFeatureMatches",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FSeagullFeatureMatches::RunTest(const FString&)
{
    FString Fixture;if(!FParse::Value(FCommandLine::Get(),TEXT("SeagullFixture="),Fixture)){AddError(TEXT("Explicit isolated loopback fixture required"));return false;}
    AddExpectedError(TEXT("Original retained; explicit reconciliation: TEMPORARY"),EAutomationExpectedErrorFlags::Contains,1);
    ADD_LATENT_AUTOMATION_COMMAND(SeagullFeatureMatches::FFlow(this));return true;
}
#endif
