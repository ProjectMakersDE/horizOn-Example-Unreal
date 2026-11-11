#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/ScopeLock.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Engine.h"
#include "Core/SeagullGameInstance.h"
#include "Horizon/SeagullHorizonManager.h"
#include "HorizonSubsystem.h"
#include "HorizonConfig.h"
#include "HttpModule.h"
#include "HttpManager.h"
#include "Interfaces/IHttpResponse.h"
#include "Managers/HorizonAuthManager.h"
#include "Examples/HorizonMarketplaceExample.h"
#include "UObject/StrongObjectPtr.h"
void RunReviewedPlayerValuesDemo(UGameInstance*);
void RunReviewedStatisticsDemo(UGameInstance*);
void RunReviewedAchievementsDemo(UGameInstance*);
void RunReviewedInboxDemo(UGameInstance*);
void RunReviewedFriendsDemo(UGameInstance*);
void ReadChatSnapshot(UHorizonSubsystem*,const FString&);
void ReadMyGroups(UHorizonSubsystem*);
void ReadInventory(UHorizonSubsystem*);
void ReadMyMatches(UHorizonSubsystem*);
void ReadWorldMap(UHorizonSubsystem*);
void ReadMarketplaceListing(UHorizonMarketplaceExample*,const FString&);
namespace DevelopGuides {
const FString Host=TEXT("http://127.0.0.1:3170");
bool Uuid(const FString& S){FGuid G;return FGuid::ParseExact(S,EGuidFormats::DigitsWithHyphens,G)&&S.Equals(G.ToString(EGuidFormats::DigitsWithHyphensLower),ESearchCase::CaseSensitive);}
bool ReadJson(const FString& Path,TSharedPtr<FJsonObject>& Out){FString Raw;return FFileHelper::LoadFileToString(Raw,*Path)&&FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Raw),Out)&&Out.IsValid();}
bool WriteNew(const FString& Path,const FString& Text){TUniquePtr<FArchive> A(IFileManager::Get().CreateFileWriter(*Path,FILEWRITE_NoReplaceExisting));if(!A)return false;FTCHARToUTF8 Bytes(*Text);A->Serialize((void*)Bytes.Get(),Bytes.Length());A->Flush();return A->Close()&&!A->IsError();}
struct FObserved {FCriticalSection Lock;int Count=0,FirstStatus=0,LastStatus=0;double Began=0,FirstSeconds=0;bool AuthDone=false,AuthOk=false;};
class FFlow final:public IAutomationLatentCommand {
    FAutomationTestBase* Test;TSharedPtr<FJsonObject> Manifest,Command;FString Feature,Route,Control,Id,ExpectedUser,Target;int Step=0,Expected=1;double Started=FPlatformTime::Seconds();bool Observing=false;TSharedRef<FObserved,ESPMode::ThreadSafe> Observed=MakeShared<FObserved,ESPMode::ThreadSafe>();TStrongObjectPtr<UHorizonMarketplaceExample> Market;
    bool Stop(const TCHAR* Error){Test->AddError(Error);return true;}
    void Detach(){if(Observing){FHttpModule::Get().GetHttpManager().SetRequestCompletedDelegate(FHttpManagerRequestCompletedDelegate());Observing=false;}}
    bool Prepare(){
        const FString ManifestPath=FPlatformMisc::GetEnvironmentVariable(TEXT("SEAGULL_DEVELOP_MANIFEST")),CommandPath=FPlatformMisc::GetEnvironmentVariable(TEXT("SEAGULL_DEVELOP_COMMAND"));
        if(!ReadJson(ManifestPath,Manifest)||!ReadJson(CommandPath,Command))return false;
        FString Stage,Base,Project,Key,Action;bool Approved=false,Allow=false,Confirm=false;
        if(!Manifest->TryGetStringField(TEXT("stage"),Stage)||Stage!=TEXT("develop")||!Manifest->TryGetBoolField(TEXT("rootApproved"),Approved)||!Approved||!Manifest->TryGetStringField(TEXT("baseUrl"),Base)||Base!=Host||!Manifest->TryGetStringField(TEXT("projectId"),Project)||!Uuid(Project)||!Manifest->TryGetStringField(TEXT("appApiKey"),Key)||Key.IsEmpty()||!Manifest->TryGetStringField(TEXT("controlDirectory"),Control)||FPaths::IsRelative(Control)||!FPaths::DirectoryExists(Control))return false;
        if(!Command->TryGetStringField(TEXT("id"),Id)||!Uuid(Id)||FPaths::ConvertRelativePathToFull(CommandPath)!=FPaths::ConvertRelativePathToFull(Control/(Id+TEXT(".json")))||!Command->TryGetStringField(TEXT("action"),Action)||Action!=TEXT("guide")||!Command->TryGetStringField(TEXT("userId"),ExpectedUser)||!Uuid(ExpectedUser)||!Command->TryGetStringField(TEXT("feature"),Feature))return false;
        const TMap<FString,FString> Routes={{TEXT("player-values"),TEXT("/player-values")},{TEXT("statistics"),TEXT("/statistics")},{TEXT("achievements"),TEXT("/achievements")},{TEXT("inbox"),TEXT("/inbox")},{TEXT("friends"),TEXT("/friends")},{TEXT("chat"),TEXT("/chat/channels/")},{TEXT("groups"),TEXT("/groups")},{TEXT("items"),TEXT("/items")},{TEXT("matches"),TEXT("/matches")},{TEXT("shared-objects"),TEXT("/shared-objects/")},{TEXT("marketplace"),TEXT("/marketplace/")}};
        const auto* Found=Routes.Find(Feature);if(!Found)return false;Route=*Found;Expected=(Feature==TEXT("statistics")||Feature==TEXT("achievements"))?2:1;
        if(Expected==2&&(!Manifest->TryGetBoolField(TEXT("allowWrites"),Allow)||!Allow||!Command->TryGetBoolField(TEXT("confirm"),Confirm)||!Confirm))return false;
        if((Feature==TEXT("chat")||Feature==TEXT("marketplace"))&&(!Command->TryGetStringField(Feature==TEXT("chat")?TEXT("channelId"):TEXT("listingId"),Target)||!Uuid(Target)))return false;
        const auto* Config=GetDefault<UHorizonConfig>();if(Config->Hosts.Num()!=1||Config->Hosts[0]!=Host||!Config->ApiKey.Equals(Key,ESearchCase::CaseSensitive))return false;
        IFileManager::Get().MakeDirectory(*(Control/TEXT("results")),true);
        return WriteNew(CommandPath+TEXT(".started"),TEXT("explicit prepared-session guide; original command retained"));
    }
public:
    explicit FFlow(FAutomationTestBase* In):Test(In){}
    ~FFlow(){Detach();}
    bool Update()override{
        if(FPlatformTime::Seconds()-Started>320)return Stop(TEXT("Prepared-session guide deadline; command and originals retained"));
        if(Step==0){if(!Prepare())return Stop(TEXT("Approved exact Develop manifest, original command, target and confirmation required"));++Step;}
        UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts())if(C.World()&&C.World()->IsGameWorld()){W=C.World();break;}if(!W)return false;
        auto* GI=Cast<USeagullGameInstance>(W->GetGameInstance());if(!GI)return false;auto* H=GI->GetHorizonManager();auto* SDK=GI->GetHorizonSubsystem();
        switch(Step){
        case 1:if(!SDK->GetHttpClient()->GetApiKey().Equals(Manifest->GetStringField(TEXT("appApiKey")),ESearchCase::CaseSensitive))return Stop(TEXT("Configured project mismatch"));H->ConnectToServer();++Step;break;
        case 2:if(!H->IsConnected())return false;H->RestoreSession([State=Observed](bool Ok){FScopeLock Lock(&State->Lock);State->AuthDone=true;State->AuthOk=Ok;},true);++Step;break;
        case 3:
            {FScopeLock Lock(&Observed->Lock);if(!Observed->AuthDone)return false;if(!Observed->AuthOk)return Stop(TEXT("Existing guest restore failed; no signup fallback"));}
            if(!SDK->Auth->IsSignedIn()||SDK->Auth->GetCurrentUser().UserId!=ExpectedUser||SDK->GetHttpClient()->GetActiveHost()!=Host)return Stop(TEXT("The same already created owned guest session is required; no signup fallback"));
            Observed->Began=FPlatformTime::Seconds();
            FHttpModule::Get().GetHttpManager().SetRequestCompletedDelegate(FHttpManagerRequestCompletedDelegate::CreateLambda([State=Observed,Prefix=Host+TEXT("/api/v1/app")+Route](const FHttpRequestRef& Request){if(!Request->GetURL().StartsWith(Prefix))return;const auto Response=Request->GetResponse();FScopeLock Lock(&State->Lock);const int Status=Response.IsValid()?Response->GetResponseCode():0;if(++State->Count==1){State->FirstStatus=Status;State->FirstSeconds=FPlatformTime::Seconds()-State->Began;}State->LastStatus=Status;}));Observing=true;
            if(Feature==TEXT("player-values"))RunReviewedPlayerValuesDemo(GI);
            else if(Feature==TEXT("statistics"))RunReviewedStatisticsDemo(GI);
            else if(Feature==TEXT("achievements"))RunReviewedAchievementsDemo(GI);
            else if(Feature==TEXT("inbox"))RunReviewedInboxDemo(GI);
            else if(Feature==TEXT("friends"))RunReviewedFriendsDemo(GI);
            else if(Feature==TEXT("chat"))ReadChatSnapshot(SDK,Target);
            else if(Feature==TEXT("groups"))ReadMyGroups(SDK);
            else if(Feature==TEXT("items"))ReadInventory(SDK);
            else if(Feature==TEXT("matches"))ReadMyMatches(SDK);
            else if(Feature==TEXT("shared-objects"))ReadWorldMap(SDK);
            else if(Feature==TEXT("marketplace")){Market.Reset(NewObject<UHorizonMarketplaceExample>());if(!Market->Bind(SDK,Manifest->GetStringField(TEXT("projectId"))))return Stop(TEXT("Existing marketplace session binding required"));ReadMarketplaceListing(Market.Get(),Target);}
            ++Step;break;
        case 4:{
            FScopeLock Lock(&Observed->Lock);if(Observed->Count<Expected)return false;
            const bool Ok=Observed->FirstStatus>=200&&Observed->FirstStatus<300&&Observed->LastStatus>=200&&Observed->LastStatus<300&&Observed->FirstSeconds<300;
            auto Result=MakeShared<FJsonObject>();Result->SetStringField(TEXT("id"),Id);Result->SetStringField(TEXT("stage"),TEXT("Develop"));Result->SetStringField(TEXT("feature"),Feature);Result->SetStringField(TEXT("userId"),ExpectedUser);Result->SetBoolField(TEXT("success"),Ok);Result->SetNumberField(TEXT("statusCode"),Observed->FirstStatus);Result->SetNumberField(TEXT("lastStatusCode"),Observed->LastStatus);Result->SetNumberField(TEXT("observedResponses"),Observed->Count);Result->SetNumberField(TEXT("secondsToHttpResponse"),Observed->FirstSeconds);Result->SetStringField(TEXT("boundary"),TEXT("Existing installed SDK/project and restored owned session; full published guide invoked explicitly; HTTP proof is separate from DTO/business reconciliation"));
            FString Json;FJsonSerializer::Serialize(Result,TJsonWriterFactory<>::Create(&Json));const FString Destination=Control/TEXT("results")/(Id+TEXT(".json"));if(!WriteNew(Destination+TEXT(".next"),Json)||IFileManager::Get().FileExists(*Destination)||!IFileManager::Get().Move(*Destination,*(Destination+TEXT(".next")),false))return Stop(TEXT("Result storage failed; original command retained"));
            if(!Ok)return Stop(TEXT("Published guide did not produce the expected successful HTTP responses"));Test->AddInfo(TEXT("Explicit full native guide reached Develop; no installation-time claim or business-state inference"));return true;
        }}return false;
    }
};}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSeagullDevelopGuides,"Seagull.Acceptance.DevelopPublicGuide",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FSeagullDevelopGuides::RunTest(const FString&){ADD_LATENT_AUTOMATION_COMMAND(DevelopGuides::FFlow(this));return true;}
#endif
