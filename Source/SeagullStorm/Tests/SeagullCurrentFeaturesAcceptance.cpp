#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "UI/SeagullFeatureLab.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Core/SeagullGameInstance.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Models/HorizonSharedObjects.h"
#include "Serialization/JsonSerializer.h"
namespace SeagullLab { bool SharedPosition(const TSharedPtr<FJsonObject>& A,FHorizonSharedObjectInput& Input); }
namespace SeagullCurrentFeatures
{
bool SyntheticFixture(const FString& Url)
{
    FString PortText=Url;if(!PortText.RemoveFromStart(TEXT("http://127.0.0.1:"),ESearchCase::CaseSensitive))return false;
    int32 Port=0;return LexTryParseString(Port,*PortText)&&LexToString(Port).Equals(PortText,ESearchCase::CaseSensitive)&&Port>0&&Port<=65535&&Port!=3170;
}
class FHello final:public IAutomationLatentCommand
{
    FAutomationTestBase* Test;double Started=FPlatformTime::Seconds();bool Checked=false,Capturing=false;TWeakObjectPtr<USeagullFeatureLab> Lab;
public:
    explicit FHello(FAutomationTestBase* T):Test(T){}
    bool Update()override
    {
        if(FPlatformTime::Seconds()-Started>30){Test->AddError(TEXT("Current-feature Hello UI did not render"));return true;}
        UWorld* World=nullptr;for(const auto& C:GEngine->GetWorldContexts())if(C.World()&&C.World()->IsGameWorld()){World=C.World();break;}if(!World)return false;
        if(!Lab.IsValid())
        {
            TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World,Widgets,USeagullFeatureLab::StaticClass(),true);
            if(Widgets.Num()!=1)return false;Lab=Cast<USeagullFeatureLab>(Widgets[0]);
        }
        if(!Checked)
        {
            const TArray<TPair<FString,FString>> Actions={{TEXT("Achievements"),TEXT("read")},{TEXT("Friends"),TEXT("read")},{TEXT("Groups"),TEXT("read")},{TEXT("Inbox"),TEXT("read")},{TEXT("Items"),TEXT("read")},{TEXT("Marketplace"),TEXT("read")},{TEXT("Matches"),TEXT("read")},{TEXT("Shared Objects"),TEXT("read-owner-page")},{TEXT("Chat"),TEXT("open-project")}};
            for(const auto& Pair:Actions){FString Error;Test->TestFalse(TEXT("Offline Hello does not invoke a feature action"),Lab->Execute(Pair.Key,Pair.Value,TEXT("{}"),Error));Test->TestEqual(TEXT("Offline screen requires the existing session"),Error,FString(TEXT("SESSION_REQUIRED")));}
            FString Error;Test->TestFalse(TEXT("Unknown CLI/method dispatch is refused"),Lab->Execute(TEXT("Friends"),TEXT("invented-method"),TEXT("{}"),Error));Test->TestEqual(TEXT("Fixed allowlist error"),Error,FString(TEXT("INVALID_ACTION")));
            for(const FName Name:{FName(TEXT("HelloReadPlayerValues")),FName(TEXT("HelloReadStatistics"))})
            {
                auto* Button=Cast<UButton>(Lab->GetWidgetFromName(Name));if(!Test->TestNotNull(TEXT("Explicit Hello SDK read button exists"),Button))continue;
                auto* Status=Cast<UTextBlock>(Lab->GetWidgetFromName(TEXT("HelloFeatureStatus")));if(!Test->TestNotNull(TEXT("Hello read result is visible"),Status))continue;
                Status->SetText(FText::FromString(TEXT("NOT_INVOKED")));Button->OnClicked.Broadcast();
                Test->TestEqual(TEXT("Hello read button preserves the existing-session guard"),Status->GetText().ToString(),FString(TEXT("SESSION_REQUIRED")));
            }
            TArray<UWidget*> Widgets;Lab->WidgetTree->GetAllWidgets(Widgets);for(auto* Widget:Widgets)if(auto* Combo=Cast<UComboBoxString>(Widget))if(Combo->FindOptionIndex(TEXT("Groups"))!=INDEX_NONE)Combo->SetSelectedOption(TEXT("Groups"));
            Checked=true;return false;
        }
        if(FPlatformTime::Seconds()-Started<2)return false;
        if(!FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
        {
            if(!Capturing){FString Directory;FParse::Value(FCommandLine::Get(),TEXT("SeagullCaptureDir="),Directory);FScreenshotRequest::RequestScreenshot(Directory/TEXT("current-feature-hello.png"),true,false);Capturing=true;return false;}
            if(FScreenshotRequest::IsScreenshotRequested())return false;
        }
        Test->AddInfo(TEXT("Actual standalone Hello rendered nine feature choices plus explicit Player Values/Statistics reads; existing-session/allowlist guards preserved without automatic backend actions."));return true;
    }
};
class FReadButtons final:public IAutomationLatentCommand
{
    FAutomationTestBase* Test;double Started=FPlatformTime::Seconds();int32 Step=0;bool AuthDone=false,AuthOk=false;TWeakObjectPtr<USeagullFeatureLab> Lab;
    void Click(const TCHAR* Name)
    {
        auto* Button=Cast<UButton>(Lab->GetWidgetFromName(Name));
        if(Test->TestNotNull(TEXT("Actual Hello read button exists"),Button))Button->OnClicked.Broadcast();
    }
public:
    explicit FReadButtons(FAutomationTestBase* T):Test(T){}
    bool Update()override
    {
        if(FPlatformTime::Seconds()-Started>40){Test->AddError(FString::Printf(TEXT("Actual Hello SDK reads timed out at step%d"),Step));return true;}
        UWorld* World=nullptr;for(const auto& Context:GEngine->GetWorldContexts())if(Context.World()&&Context.World()->IsGameWorld()){World=Context.World();break;}if(!World)return false;
        auto* GI=Cast<USeagullGameInstance>(World->GetGameInstance());if(!GI)return false;auto* Manager=GI->GetHorizonManager();
        if(!Lab.IsValid()){TArray<UUserWidget*> Widgets;UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World,Widgets,USeagullFeatureLab::StaticClass(),true);if(Widgets.Num()!=1)return false;Lab=Cast<USeagullFeatureLab>(Widgets[0]);}
        auto* Status=Cast<UTextBlock>(Lab->GetWidgetFromName(TEXT("HelloFeatureStatus")));if(!Status)return false;const FString Text=Status->GetText().ToString();
        switch(Step)
        {
        case 0:Manager->ConnectToServer();++Step;break;
        case 1:if(!Manager->IsConnected())return false;Manager->SignInEmail(TEXT("hello@example.invalid"),TEXT("synthetic-password"),[this](bool Ok){AuthDone=true;AuthOk=Ok;});++Step;break;
        case 2:
            if(!AuthDone)return false;if(!AuthOk){Test->AddError(TEXT("Synthetic existing Hello session failed"));return true;}GI->BeginOnlineSession(false);
            Click(TEXT("HelloReadPlayerValues"));++Step;break;
        case 3:
            if(!Text.Contains(TEXT("coins"))||!Text.Contains(TEXT("42")))return false;
            Test->AddInfo(TEXT("Actual Hello Player Values button rendered the typed authoritative coins=42 snapshot."));Click(TEXT("HelloReadStatistics"));++Step;break;
        case 4:
            if(!Text.Contains(TEXT("kills"))||!Text.Contains(TEXT("201")))return false;
            Test->AddInfo(TEXT("Actual Hello Statistics button rendered the typed authoritative kills=201 snapshot."));Click(TEXT("HelloReadPlayerValues"));++Step;break;
        case 5:if(!Text.Contains(TEXT("PV_READ_DENIED")))return false;Click(TEXT("HelloReadStatistics"));++Step;break;
        case 6:
            if(!Text.Contains(TEXT("STATISTICS_READ_DENIED")))return false;
            Test->AddInfo(TEXT("Both SDK read errors are visible in the actual Hello UI; no feature POST or invented state."));return true;
        }
        return false;
    }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSeagullCurrentFeaturesHello,"Seagull.Acceptance.CurrentFeaturesHello",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FSeagullCurrentFeaturesHello::RunTest(const FString&)
{
    for(const FString Json:{TEXT("{}"),TEXT("{\"position\":null}"),TEXT("{\"position\":{\"x\":\"10\",\"y\":\"-10\"}}"),TEXT("{\"position\":{\"x\":\"-9007199254740991\",\"y\":\"9007199254740991\"}}")})
    {
        TSharedPtr<FJsonObject> Args;TestTrue(TEXT("Position fixture parses"),FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),Args));FHorizonSharedObjectInput Input;Input.Data={0,255};
        TestTrue(TEXT("Optional position accepts exact canonical coordinates"),SeagullLab::SharedPosition(Args,Input));TestTrue(TEXT("Position leaves caller payload unchanged"),Input.Data==TArray<uint8>({0,255}));
        const auto Position=Args->TryGetField(TEXT("position"));const bool Present=Position&&!Position->IsNull();TestEqual(TEXT("Only a supplied position enables the SDK field"),Input.bHasPosition,Present);
        if(Json.Contains(TEXT("\"10\""))){TestEqual(TEXT("X preserved"),Input.Position.X,int64(10));TestEqual(TEXT("Y preserved"),Input.Position.Y,int64(-10));}
    }
    for(const FString Position:{TEXT("[]"),TEXT("false"),TEXT("\"10,10\""),TEXT("{}"),TEXT("{\"x\":\"10\"}"),TEXT("{\"x\":10,\"y\":10}"),TEXT("{\"x\":\"1.5\",\"y\":\"0\"}"),TEXT("{\"x\":\"01\",\"y\":\"0\"}"),TEXT("{\"x\":\"9007199254740992\",\"y\":\"0\"}"),TEXT("{\"x\":\"0\",\"y\":\"0\",\"z\":\"0\"}")})
    {
        TSharedPtr<FJsonObject> Args;TestTrue(TEXT("Invalid position fixture parses"),FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(TEXT("{\"position\":")+Position+TEXT("}")),Args));FHorizonSharedObjectInput Input;
        TestFalse(TEXT("Malformed position refused before SDK dispatch"),SeagullLab::SharedPosition(Args,Input));TestFalse(TEXT("Rejected position does not enable SDK field"),Input.bHasPosition);
    }
    ADD_LATENT_AUTOMATION_COMMAND(SeagullCurrentFeatures::FHello(this));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSeagullCurrentFeaturesHelloReads,"Seagull.Acceptance.HelloAuthoritativeReads",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FSeagullCurrentFeaturesHelloReads::RunTest(const FString&)
{
    TestTrue(TEXT("Canonical isolated fixture port accepted"),SeagullCurrentFeatures::SyntheticFixture(TEXT("http://127.0.0.1:19999")));
    for(const FString Url:{TEXT("http://127.0.0.1:3170"),TEXT("http://127.0.0.1:3170/"),TEXT("http://127.0.0.1:3170/path"),TEXT("http://127.0.0.1:19999/path"),TEXT("http://127.0.0.1:19999?next=3170"),TEXT("http://127.0.0.1:03170"),TEXT("http://127.0.0.1:65536"),TEXT("https://horizon.pm")})TestFalse(TEXT("Develop/noncanonical/nonloopback fixture rejected before native auth"),SeagullCurrentFeatures::SyntheticFixture(Url));
    FString Fixture;if(!FParse::Value(FCommandLine::Get(),TEXT("SeagullFixture="),Fixture)||!SeagullCurrentFeatures::SyntheticFixture(Fixture)){AddError(TEXT("Isolated synthetic loopback fixture required; this test is not a Develop acceptance entry"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(SeagullCurrentFeatures::FReadButtons(this));return true;
}
#endif
