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
namespace SeagullCurrentFeatures
{
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
            TArray<UWidget*> Widgets;Lab->WidgetTree->GetAllWidgets(Widgets);for(auto* Widget:Widgets)if(auto* Combo=Cast<UComboBoxString>(Widget))if(Combo->FindOptionIndex(TEXT("Groups"))!=INDEX_NONE)Combo->SetSelectedOption(TEXT("Groups"));
            Checked=true;return false;
        }
        if(FPlatformTime::Seconds()-Started<2)return false;
        if(!FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
        {
            if(!Capturing){FString Directory;FParse::Value(FCommandLine::Get(),TEXT("SeagullCaptureDir="),Directory);FScreenshotRequest::RequestScreenshot(Directory/TEXT("current-feature-hello.png"),true,false);Capturing=true;return false;}
            if(FScreenshotRequest::IsScreenshotRequested())return false;
        }
        Test->AddInfo(TEXT("Actual standalone Hello rendered all nine current feature choices; existing-session/allowlist guards preserved without automatic backend actions."));return true;
    }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSeagullCurrentFeaturesHello,"Seagull.Acceptance.CurrentFeaturesHello",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FSeagullCurrentFeaturesHello::RunTest(const FString&){ADD_LATENT_AUTOMATION_COMMAND(SeagullCurrentFeatures::FHello(this));return true;}
#endif
