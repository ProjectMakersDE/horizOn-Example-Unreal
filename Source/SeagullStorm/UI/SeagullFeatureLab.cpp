#include "UI/SeagullFeatureLab.h"
#include "Examples/HorizonChatExampleWidget.h"
#include "GameFramework/PlayerController.h"
#include "UI/SeagullWidgetStyles.h"
#include "Core/SeagullTypes.h"
#include "Core/SeagullGameInstance.h"
#include "Horizon/SeagullHorizonManager.h"
#include "HorizonSubsystem.h"
#include "Managers/HorizonAuthManager.h"
#include "Managers/HorizonMarketplaceManager.h"
#include "Managers/HorizonPlayerValuesManager.h"
#include "Managers/HorizonStatisticsManager.h"
#include "Examples/HorizonFriendsExample.h"
#include "Examples/HorizonGroupsExample.h"
#include "Examples/HorizonItemsExample.h"
#include "Examples/HorizonSharedObjectsExample.h"
#include "Examples/HorizonMatchesExample.h"
#include "Examples/HorizonInboxExample.h"
#include "Examples/HorizonAchievementsExample.h"
#include "Examples/HorizonChatExample.h"
#include "Examples/HorizonMarketplaceExample.h"
#include "Examples/HorizonExampleIdentity.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Base64.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"

namespace SeagullLab
{
const TMap<FString,TArray<FString>>& Actions()
{
    static const TMap<FString,TArray<FString>> Map={
        {TEXT("Achievements"),{TEXT("read"),TEXT("unlock"),TEXT("deliver-legacy"),TEXT("retry-original")}},
        {TEXT("Friends"),{TEXT("read"),TEXT("find"),TEXT("request"),TEXT("accept"),TEXT("decline"),TEXT("cancel"),TEXT("remove"),TEXT("block"),TEXT("unblock"),TEXT("retry-original"),TEXT("acknowledge-after-refresh")}},
        {TEXT("Groups"),{TEXT("read"),TEXT("create"),TEXT("update"),TEXT("join"),TEXT("request"),TEXT("invite"),TEXT("accept-invite"),TEXT("decline-invite"),TEXT("cancel-invite"),TEXT("accept-request"),TEXT("decline-request"),TEXT("cancel-request"),TEXT("leave"),TEXT("remove"),TEXT("role"),TEXT("transfer"),TEXT("delete"),TEXT("retry-original"),TEXT("acknowledge-after-refresh")}},
        {TEXT("Inbox"),{TEXT("read"),TEXT("mark-read"),TEXT("remove"),TEXT("claim-original"),TEXT("retry-original")}},
        {TEXT("Items"),{TEXT("read"),TEXT("purchase-offer"),TEXT("equip"),TEXT("adjust"),TEXT("discard"),TEXT("retry-original"),TEXT("retry-original-after-offer-change"),TEXT("acknowledge-after-refresh")}},
        {TEXT("Marketplace"),{TEXT("read-limits"),TEXT("read-rules-page"),TEXT("read"),TEXT("read-listings-page"),TEXT("read-exact-listing"),TEXT("read-inventory"),TEXT("review-create"),TEXT("review-buy"),TEXT("review-cancel"),TEXT("confirm-reviewed-trade"),TEXT("retry-original"),TEXT("look-up-original"),TEXT("retry-return"),TEXT("claim-return-original")}},
        {TEXT("Matches"),{TEXT("read"),TEXT("find-open-page"),TEXT("create"),TEXT("join"),TEXT("decline"),TEXT("start"),TEXT("turn"),TEXT("timeout"),TEXT("leave"),TEXT("cancel"),TEXT("result"),TEXT("recover-original-ranked-source"),TEXT("retry-original"),TEXT("submit-original-ranked-result"),TEXT("confirm-recovered-create"),TEXT("acknowledge-after-refresh")}},
        {TEXT("Shared Objects"),{TEXT("read-owner-page"),TEXT("query-rectangle-page"),TEXT("query-opponents"),TEXT("create"),TEXT("update"),TEXT("delete"),TEXT("retry-original"),TEXT("confirm-recovered-create"),TEXT("acknowledge-after-refresh")}},
        {TEXT("Chat"),{TEXT("open-project"),TEXT("show-chat-ui"),TEXT("send-original-text"),TEXT("look-up-original"),TEXT("retry-original"),TEXT("report-original"),TEXT("close-channel")}}
    };return Map;
}
FString Text(const TSharedPtr<FJsonObject>& A,const TCHAR* Key,const FString& Fallback=TEXT("")){FString V;return A->TryGetStringField(Key,V)?V:Fallback;}
bool Integer(const TSharedPtr<FJsonObject>& A,const TCHAR* Key,int64& Value,int64 Default=0)
{
    FString V;if(!A->HasField(Key)){Value=Default;return true;}if(!A->TryGetStringField(Key,V)||V.IsEmpty()||!LexTryParseString(Value,*V))return false;
    return LexToString(Value)==V&&Value>=-9007199254740991LL&&Value<=9007199254740991LL;
}
bool Flag(const TSharedPtr<FJsonObject>& A){bool V=false;A->TryGetBoolField(TEXT("flag"),V);return V;}
bool MatchIds(const TSharedPtr<FJsonObject>& A,TArray<FString>& Out)
{
    if(!A->HasField(TEXT("ids")))return true;
    const TArray<TSharedPtr<FJsonValue>>* Values;if(!A->TryGetArrayField(TEXT("ids"),Values))return false;
    TSet<FString> Seen;for(const auto& Value:*Values)
    {
        FString Id;FGuid Parsed;if(!Value||!Value->TryGetString(Id)||Id.Len()!=36||!FGuid::ParseExact(Id,EGuidFormats::DigitsWithHyphens,Parsed)||Seen.Contains(Id.ToLower()))return false;
        Id=Parsed.ToString(EGuidFormats::DigitsWithHyphensLower);Seen.Add(Id);Out.Add(Id);
    }
    return true; // The SDK owns route-specific cardinality, ordering and participant validation.
}
template<typename T>FString Json(const T& Value){FString Out;FJsonObjectConverter::UStructToJsonObjectString(T::StaticStruct(),&Value,Out,0,0);return Out;}
}

void USeagullFeatureLab::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    auto* Panel=SeagullWidgetStyles::MakePanel(WidgetTree,SeagullColors::PanelBG,24.f);WidgetTree->RootWidget=Panel;
    auto* Scroll=WidgetTree->ConstructWidget<UScrollBox>();Panel->SetContent(Scroll);auto* Column=WidgetTree->ConstructWidget<UVerticalBox>();Scroll->AddChild(Column);
    SeagullWidgetStyles::AddRow(Column,SeagullWidgetStyles::MakeText(WidgetTree,TEXT("horizOn / CURRENT FEATURES + HELLO"),24,SeagullColors::Orange));
    auto* Intro=SeagullWidgetStyles::MakeText(WidgetTree,TEXT("Existing player only. Every action is explicit. Official SDK journals retain originals after uncertainty; no local rewards or historical item reconstruction. Configure catalogs first. Numeric inputs use decimal strings."),16,SeagullColors::LightText);Intro->SetAutoWrapText(true);SeagullWidgetStyles::AddRow(Column,Intro);
    auto* Auth=WidgetTree->ConstructWidget<UWrapBox>();SeagullWidgetStyles::AddRow(Column,Auth);
    auto* Connection=SeagullWidgetStyles::MakeTextButton(WidgetTree,TEXT("Connect (no signup)"),SeagullColors::Teal,SeagullColors::LightText,16);Connection->OnClicked.AddDynamic(this,&USeagullFeatureLab::Connect);Auth->AddChildToWrapBox(Connection);
    auto* Restoration=SeagullWidgetStyles::MakeTextButton(WidgetTree,TEXT("Restore original identity"),SeagullColors::Teal,SeagullColors::LightText,16);Restoration->OnClicked.AddDynamic(this,&USeagullFeatureLab::Restore);Auth->AddChildToWrapBox(Restoration);
    auto* Guest=SeagullWidgetStyles::MakeTextButton(WidgetTree,TEXT("Create guest explicitly"),SeagullColors::Teal,SeagullColors::LightText,16);Guest->OnClicked.AddDynamic(this,&USeagullFeatureLab::CreateGuest);Auth->AddChildToWrapBox(Guest);
    auto* Reads=WidgetTree->ConstructWidget<UWrapBox>();SeagullWidgetStyles::AddRow(Column,Reads);
    auto* Values=SeagullWidgetStyles::MakeTextButton(WidgetTree,TEXT("Read authoritative player values"),SeagullColors::Teal,SeagullColors::LightText,16,TEXT("HelloReadPlayerValues"));Values->OnClicked.AddDynamic(this,&USeagullFeatureLab::ReadPlayerValues);Reads->AddChildToWrapBox(Values);
    auto* Statistics=SeagullWidgetStyles::MakeTextButton(WidgetTree,TEXT("Read statistics"),SeagullColors::Teal,SeagullColors::LightText,16,TEXT("HelloReadStatistics"));Statistics->OnClicked.AddDynamic(this,&USeagullFeatureLab::ReadStatistics);Reads->AddChildToWrapBox(Statistics);
    ProjectInput=SeagullWidgetStyles::MakeTextInput(WidgetTree,TEXT("Stable project UUID from project settings (Marketplace only; bound once per key)"));SeagullWidgetStyles::AddRow(Column,ProjectInput);
    FeatureInput=WidgetTree->ConstructWidget<UComboBoxString>();for(const auto& Pair:SeagullLab::Actions())FeatureInput->AddOption(Pair.Key);FeatureInput->OnSelectionChanged.AddDynamic(this,&USeagullFeatureLab::SelectFeature);SeagullWidgetStyles::AddRow(Column,FeatureInput);
    ActionInput=WidgetTree->ConstructWidget<UComboBoxString>();SeagullWidgetStyles::AddRow(Column,ActionInput);
    ArgumentsInput=WidgetTree->ConstructWidget<UMultiLineEditableTextBox>();FTextBlockStyle ArgumentStyle=ArgumentsInput->WidgetStyle.TextStyle;ArgumentStyle.SetFont(SeagullWidgetStyles::GetPixelFont(16));ArgumentStyle.SetColorAndOpacity(FSlateColor(SeagullColors::DarkText));ArgumentsInput->SetTextStyle(ArgumentStyle);ArgumentsInput->SetForegroundColor(SeagullColors::DarkText);ArgumentsInput->SetText(FText::FromString(TEXT("{\"id\":\"\",\"target\":\"\",\"key\":\"\",\"requestId\":\"\",\"revision\":\"0\",\"name\":\"Seagulls\",\"description\":\"\",\"mode\":\"open\",\"dataJson\":\"{}\",\"base64\":\"\",\"collection\":\"levels\",\"price\":\"10\",\"duration\":\"3600\",\"amount\":\"-1\",\"text\":\"Hello from Seagull Storm\",\"flag\":false}")));SeagullWidgetStyles::AddRow(Column,ArgumentsInput);
    auto* Run=SeagullWidgetStyles::MakeTextButton(WidgetTree,TEXT("Run selected explicit action"),SeagullColors::Orange,SeagullColors::DarkText,18);Run->OnClicked.AddDynamic(this,&USeagullFeatureLab::RunSelected);SeagullWidgetStyles::AddRow(Column,Run);
    auto* Snapshot=SeagullWidgetStyles::MakeTextButton(WidgetTree,TEXT("Show current cached snapshot (no request)"),SeagullColors::Teal,SeagullColors::LightText,16);Snapshot->OnClicked.AddDynamic(this,&USeagullFeatureLab::ShowSnapshot);SeagullWidgetStyles::AddRow(Column,Snapshot);
    Status=SeagullWidgetStyles::MakeText(WidgetTree,TEXT("No feature action has run. Async actions log exact errors; invoke Show snapshot after completion."),16,SeagullColors::LightText,TEXT("HelloFeatureStatus"));Status->SetAutoWrapText(true);SeagullWidgetStyles::AddRow(Column,Status);
    auto* Back=SeagullWidgetStyles::MakeTextButton(WidgetTree,TEXT("Back to game"),SeagullColors::Orange,SeagullColors::DarkText,18);Back->OnClicked.AddDynamic(this,&USeagullFeatureLab::Close);SeagullWidgetStyles::AddRow(Column,Back);
    FeatureInput->SetSelectedOption(TEXT("Achievements"));
    SetMenuFirstFocus(FeatureInput);
    SetMenuBack(FSimpleDelegate::CreateUObject(this,&USeagullFeatureLab::Close));
}
void USeagullFeatureLab::SelectFeature(FString Selected,ESelectInfo::Type){ActionInput->ClearOptions();if(const auto* Actions=SeagullLab::Actions().Find(Selected)){for(const auto& A:*Actions)ActionInput->AddOption(A);ActionInput->SetSelectedIndex(0);}}
UHorizonSubsystem* USeagullFeatureLab::Session()const
{
    auto* GI=Cast<USeagullGameInstance>(GetGameInstance());auto* S=GI?GI->GetHorizonSubsystem():nullptr;
    return GI&&!GI->bPracticeMode&&S&&S->Auth&&S->Auth->IsSignedIn()&&S->Auth->GetCurrentUser().AccessToken==S->GetHttpClient()->GetSessionToken()?S:nullptr;
}
void USeagullFeatureLab::RunSelected(){FString Error;const bool Accepted=Execute(FeatureInput->GetSelectedOption(),ActionInput->GetSelectedOption(),ArgumentsInput->GetText().ToString(),Error);Status->SetText(FText::FromString(Accepted?TEXT("Explicit action invoked. Wait for SDK completion, then show the snapshot. Invocation alone is not a committed result; any uncertain original remains protected."):Error));}
void USeagullFeatureLab::ReadPlayerValues()
{
    auto* SDK=Session();if(!SDK){Status->SetText(FText::FromString(TEXT("SESSION_REQUIRED")));return;}
    const uint64 Context=SDK->GetHttpClient()->GetRequestContextEpoch();const FString User=SDK->Auth->GetCurrentUser().UserId;
    Status->SetText(FText::FromString(TEXT("Reading authoritative player values...")));
    SDK->PlayerValues->GetState(User,FOnPlayerValuesComplete::CreateWeakLambda(this,[this,Context,User](bool Ok,const FHorizonPlayerValuesState& State,const FString& Code,const FString&)
    {
        auto* Current=Session();if(!IsInViewport()||!Current||Current->GetHttpClient()->GetRequestContextEpoch()!=Context||Current->Auth->GetCurrentUser().UserId!=User)return;
        Status->SetText(FText::FromString(Ok?SeagullLab::Json(State):TEXT("Player values read refused: ")+Code+TEXT(". Saved originals remain protected.")));
    }));
}
void USeagullFeatureLab::ReadStatistics()
{
    auto* SDK=Session();if(!SDK){Status->SetText(FText::FromString(TEXT("SESSION_REQUIRED")));return;}
    const uint64 Context=SDK->GetHttpClient()->GetRequestContextEpoch();const FString User=SDK->Auth->GetCurrentUser().UserId;
    Status->SetText(FText::FromString(TEXT("Reading authoritative statistics...")));
    SDK->Statistics->GetStatistics(FOnStatisticsLoaded::CreateWeakLambda(this,[this,Context,User](bool Ok,const FHorizonStatisticsSnapshot& State,const FString& Code,const FString&)
    {
        auto* Current=Session();if(!IsInViewport()||!Current||Current->GetHttpClient()->GetRequestContextEpoch()!=Context||Current->Auth->GetCurrentUser().UserId!=User)return;
        Status->SetText(FText::FromString(Ok?SeagullLab::Json(State):TEXT("Statistics read refused: ")+Code+TEXT(". Saved originals remain protected.")));
    }));
}
bool USeagullFeatureLab::BindMarketplace(FString& Error)
{
    auto* S=Session();if(!S){Error=TEXT("SESSION_REQUIRED");return false;}
    const FString Hash=UHorizonExampleIdentity::Project(UHorizonExampleIdentity::ProjectKey(S)),Slot=TEXT("seagull-project-binding-")+Hash;
    USeagullProjectBinding* Binding=nullptr;
    if(UGameplayStatics::DoesSaveGameExist(Slot,0))
    {Binding=Cast<USeagullProjectBinding>(UGameplayStatics::LoadGameFromSlot(Slot,0));if(!Binding||Binding->KeyHash!=Hash){Error=TEXT("PROJECT_BINDING_RECOVERY_REQUIRED");return false;}}
    FString Requested=ProjectInput->GetText().ToString();if(Requested.IsEmpty()&&Binding)Requested=Binding->Project;
    FGuid Id;if(!FGuid::ParseExact(Requested,EGuidFormats::DigitsWithHyphens,Id)){Error=TEXT("STABLE_PROJECT_UUID_REQUIRED");return false;}Requested=Id.ToString(EGuidFormats::DigitsWithHyphens).ToLower();
    if(Binding&&Binding->Project!=Requested){Error=TEXT("PROJECT_BINDING_MISMATCH");return false;}
    if(!Binding){Binding=Cast<USeagullProjectBinding>(UGameplayStatics::CreateSaveGameObject(USeagullProjectBinding::StaticClass()));Binding->Project=Requested;Binding->KeyHash=Hash;if(!UGameplayStatics::SaveGameToSlot(Binding,Slot,0)){Error=TEXT("PROJECT_BINDING_STORAGE_ERROR");return false;}auto* Saved=Cast<USeagullProjectBinding>(UGameplayStatics::LoadGameFromSlot(Slot,0));if(!Saved||Saved->Project!=Requested||Saved->KeyHash!=Hash){Error=TEXT("PROJECT_BINDING_STORAGE_ERROR");return false;}}
    ProjectInput->SetText(FText::FromString(Requested));
    if(!Marketplace){Marketplace=NewObject<UHorizonMarketplaceExample>(this);Marketplace->OnAction.AddDynamic(this,&USeagullFeatureLab::MarketplaceFinished);}
    if(BoundMarketProject==Requested)return true;
    if(!Marketplace->Bind(S,Requested)){Error=TEXT("MARKETPLACE_BINDING_REQUIRED");return false;}BoundMarketProject=Requested;return true;
}
bool USeagullFeatureLab::Execute(const FString& Feature,const FString& Action,const FString& Arguments,FString& Error)
{
    const auto* Allowed=SeagullLab::Actions().Find(Feature);if(!Allowed||!Allowed->Contains(Action)){Error=TEXT("INVALID_ACTION");return false;}
    auto* S=Session();if(!S){Error=TEXT("SESSION_REQUIRED");return false;}
    if(LastContext!=S->GetHttpClient()->GetRequestContextEpoch()){ClearRecipes();LastContext=S->GetHttpClient()->GetRequestContextEpoch();}
    TSharedPtr<FJsonObject> A;if(!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Arguments),A)||!A){Error=TEXT("ARGUMENTS_MUST_BE_JSON_OBJECT");return false;}
    const FString Id=SeagullLab::Text(A,TEXT("id")),Target=SeagullLab::Text(A,TEXT("target")),Key=SeagullLab::Text(A,TEXT("key")),Request=SeagullLab::Text(A,TEXT("requestId")),Collection=SeagullLab::Text(A,TEXT("collection"),TEXT("levels"));
    int64 Revision,Price,Duration,Amount,Capacity,Strength,Turn;
    if(!SeagullLab::Integer(A,TEXT("revision"),Revision)||!SeagullLab::Integer(A,TEXT("price"),Price,10)||!SeagullLab::Integer(A,TEXT("duration"),Duration,3600)||!SeagullLab::Integer(A,TEXT("amount"),Amount,-1)||!SeagullLab::Integer(A,TEXT("capacity"),Capacity,2)||!SeagullLab::Integer(A,TEXT("strength"),Strength)||!SeagullLab::Integer(A,TEXT("turnSeconds"),Turn,60)||Capacity>MAX_int32||Capacity<0||Turn>MAX_int32||Turn<0){Error=TEXT("CANONICAL_DECIMAL_STRING_REQUIRED");return false;}
    TArray<uint8> Data;if(!FBase64::Decode(SeagullLab::Text(A,TEXT("base64")),Data)){Error=TEXT("INVALID_BASE64");return false;}
    const bool Flag=SeagullLab::Flag(A);
    if(Feature==TEXT("Friends"))
    {
        if(!Friends)Friends=GetWorld()->SpawnActor<AHorizonFriendsExample>();
        if(Action==TEXT("read"))Friends->LoadFriends();else if(Action==TEXT("find"))Friends->FindPlayer(Id);else if(Action==TEXT("retry-original"))Friends->RetrySavedAction();else if(Action==TEXT("acknowledge-after-refresh"))return Friends->AcknowledgePendingAfterRefresh();else Friends->StartAction(Action,Target,Revision,Request);
    }
    else if(Feature==TEXT("Groups"))
    {
        if(!Groups)Groups=GetWorld()->SpawnActor<AHorizonGroupsExample>();
        FHorizonGroupMetadata Metadata;Metadata.Name=SeagullLab::Text(A,TEXT("name"));Metadata.Description=SeagullLab::Text(A,TEXT("description"));Metadata.JoinMode=SeagullLab::Text(A,TEXT("mode"),TEXT("open"));
        if(!FHorizonGroupJsonData::TryFromJson(SeagullLab::Text(A,TEXT("dataJson"),TEXT("{}")),Metadata.Data)){Error=TEXT("INVALID_GROUP_DATA");return false;}
        if(Action==TEXT("read"))Groups->Refresh(Id);else if(Action==TEXT("retry-original"))Groups->RetrySavedAction();else if(Action==TEXT("acknowledge-after-refresh"))return Groups->AcknowledgePendingAfterRefresh();else Groups->StartAction(Action,Id,Target,Revision,Request,SeagullLab::Text(A,TEXT("role")),Metadata);
    }
    else if(Feature==TEXT("Items"))
    {
        if(!Items)Items=GetWorld()->SpawnActor<AHorizonItemsExample>();
        if(Action==TEXT("read"))Items->Refresh();else if(Action==TEXT("purchase-offer"))Items->PurchaseItemOffer(Key);else if(Action.StartsWith(TEXT("retry-original")))Items->RetrySavedAction(Action==TEXT("retry-original-after-offer-change"));else if(Action==TEXT("acknowledge-after-refresh"))return Items->AcknowledgeItemActionAfterRefresh();else{TArray<FHorizonItemPropertyDelta> Changes;if(Action==TEXT("adjust")){FHorizonItemPropertyDelta Delta;Delta.Key=Key;Delta.Amount=Amount;Changes.Add(Delta);}Items->StartItemAction(Action,Id,Revision,Changes,Flag);}
    }
    else if(Feature==TEXT("Inbox"))
    {
        if(!Inbox)Inbox=GetWorld()->SpawnActor<AHorizonInboxExample>();
        if(Action==TEXT("read"))Inbox->LoadInbox();else if(Action==TEXT("mark-read"))Inbox->MarkRead(Id);else if(Action==TEXT("remove"))Inbox->Remove(Id);else Inbox->Claim(Action==TEXT("retry-original")?Inbox->GetPendingClaimEntryId():Id);
    }
    else if(Feature==TEXT("Achievements"))
    {
        if(!Achievements)Achievements=GetWorld()->SpawnActor<AHorizonAchievementsExample>();
        if(Action==TEXT("read"))Achievements->LoadAchievements();else if(Action==TEXT("unlock"))Achievements->UnlockAchievement(Key);else if(Action==TEXT("retry-original"))Achievements->RetrySavedDelivery();else Achievements->DeliverPendingRewards({Key});
    }
    else if(Feature==TEXT("Marketplace"))
    {
        if(!BindMarketplace(Error))return false;
        if(Action==TEXT("read-limits"))Marketplace->ReadLimits();else if(Action==TEXT("read-rules-page"))Marketplace->ReadRulesPage(Key,A->HasField(TEXT("revision"))?Revision:-1);else if(Action==TEXT("read"))Marketplace->ReadMarketplace();else if(Action==TEXT("read-listings-page"))Marketplace->ReadListings(Key,0,9007199254740991LL,Request);else if(Action==TEXT("read-exact-listing"))Marketplace->ReadListing(Id);else if(Action==TEXT("read-inventory"))Marketplace->ReadInventory();else if(Action==TEXT("review-create"))return Marketplace->ReviewCreate(Id,Price,Duration);else if(Action==TEXT("review-buy"))return Marketplace->ReviewBuy(Id);else if(Action==TEXT("review-cancel"))return Marketplace->ReviewCancel(Id);else if(Action==TEXT("confirm-reviewed-trade"))Marketplace->ConfirmReviewedTrade();else if(Action==TEXT("retry-original"))Marketplace->RetryPending();else if(Action==TEXT("look-up-original"))Marketplace->RecoverOriginalOperation();else if(Action==TEXT("retry-return"))Marketplace->RetryReturnDelivery(Id);else{if(!Inbox)Inbox=GetWorld()->SpawnActor<AHorizonInboxExample>();Inbox->Claim(Id);}
    }
    else if(Feature==TEXT("Matches"))
    {
        TArray<FString> Ids;if(!SeagullLab::MatchIds(A,Ids)||((Action==TEXT("start")||Action==TEXT("result"))&&!A->HasField(TEXT("ids")))){Error=TEXT("MATCH_IDS_ARRAY_REQUIRED");return false;}
        if(!Matches)Matches=GetWorld()->SpawnActor<AHorizonMatchesExample>();
        if(Action==TEXT("read"))Matches->Refresh(Id);else if(Action==TEXT("find-open-page"))Matches->FindOpen(0,9007199254740991LL,Request);else if(Action==TEXT("create")){FHorizonMatchCreateInput Input;Input.Name=SeagullLab::Text(A,TEXT("name"));Input.Capacity=static_cast<int32>(Capacity);Input.Strength=Strength;Input.TurnSeconds=static_cast<int32>(Turn);Input.Visibility=SeagullLab::Text(A,TEXT("visibility"),TEXT("private"));Input.TimeoutPolicy=SeagullLab::Text(A,TEXT("timeoutPolicy"),TEXT("skip"));Input.Data=Data;Input.bRanked=Flag;Input.InviteeIds=Ids;if(!A->HasField(TEXT("ids"))&&!Target.IsEmpty())Input.InviteeIds.Add(Target);Matches->CreateLobby(Input);}else if(Action==TEXT("recover-original-ranked-source"))Matches->StartRankedFromPendingResult(Id);else if(Action==TEXT("retry-original"))Matches->RetryOriginal();else if(Action==TEXT("submit-original-ranked-result"))Matches->SubmitRankedResult(Amount,Data);else if(Action==TEXT("confirm-recovered-create"))return Matches->ConfirmRecoveredCreate(Id);else if(Action==TEXT("acknowledge-after-refresh"))return Matches->AcknowledgeAfterRefresh();else Matches->StartAction(Action,Id,Revision,Data,Ids,Flag);
    }
    else if(Feature==TEXT("Shared Objects"))
    {
        if(!Shared)Shared=GetWorld()->SpawnActor<AHorizonSharedObjectsExample>();
        FHorizonSharedObjectInput Input;Input.Data=Data;
        if(Action==TEXT("read-owner-page"))Shared->RefreshOwned(Collection,Target,Request);else if(Action==TEXT("query-rectangle-page"))Shared->QueryRectangle(Collection,0,100,0,100,Request);else if(Action==TEXT("query-opponents"))Shared->QueryOpponents(Collection,Key.IsEmpty()?TEXT("strength"):Key,0,100);else if(Action==TEXT("create"))Shared->CreateMarker(Input);else if(Action==TEXT("update"))Shared->UpdateMarker(Id,Revision,Input);else if(Action==TEXT("delete"))Shared->DeleteMarker(Id,Revision);else if(Action==TEXT("retry-original"))Shared->RetryOriginal();else if(Action==TEXT("confirm-recovered-create"))return Shared->ConfirmRecoveredCreate(Id);else return Shared->AcknowledgeAfterRefresh();
    }
    else if(Feature==TEXT("Chat"))
    {
        if(!Chat)Chat=GetWorld()->SpawnActor<AHorizonChatExample>();
        if(Action==TEXT("open-project")){FHorizonChatOpenInput Input;Input.Kind=TEXT("project");Chat->Open(Input);}else if(Action==TEXT("show-chat-ui"))
        {
            if(!ChatPanel||!ChatPanel->IsInViewport())
            {
                ChatPanel=CreateWidget<USeagullChatPanel>(GetOwningPlayer());
                if(ChatPanel){ChatPanel->BindExample(Chat);ChatPanel->AddToViewport(200);}
            }
        }
        else if(Action==TEXT("send-original-text"))Chat->SendText(SeagullLab::Text(A,TEXT("text")));else if(Action==TEXT("look-up-original"))Chat->ReconcileOriginal();else if(Action==TEXT("retry-original"))Chat->RetryOriginal();else if(Action==TEXT("report-original"))Chat->Report(Id,SeagullLab::Text(A,TEXT("text")));else Chat->CloseChannel();
    }
    return true;
}
void USeagullFeatureLab::ShowSnapshot()
{
    if(!Session()){Status->SetText(FText::FromString(TEXT("SESSION_REQUIRED")));return;}
    FString Text;const FString Feature=FeatureInput->GetSelectedOption();
    if(Feature==TEXT("Friends")&&Friends)Text=SeagullLab::Json(Friends->GetCurrentFriends());else if(Feature==TEXT("Groups")&&Groups)Text=SeagullLab::Json(Groups->GetGroups())+TEXT("\n")+SeagullLab::Json(Groups->GetGroup());else if(Feature==TEXT("Inbox")&&Inbox)Text=SeagullLab::Json(Inbox->GetCurrentInbox());else if(Feature==TEXT("Items")&&Items)Text=SeagullLab::Json(Items->GetInventory());else if(Feature==TEXT("Matches")&&Matches)Text=Matches->LastErrorCode+TEXT("\n")+SeagullLab::Json(Matches->GetMatches())+TEXT("\n")+SeagullLab::Json(Matches->GetMatch());else if(Feature==TEXT("Shared Objects")&&Shared)Text=SeagullLab::Json(Shared->GetUsage())+TEXT("\n")+SeagullLab::Json(Shared->GetObjects());else if(Feature==TEXT("Achievements")&&Achievements)Text=SeagullLab::Json(Achievements->GetCurrentAchievements());else if(Feature==TEXT("Chat")&&Chat)Text=Chat->LastErrorCode+TEXT("\n")+SeagullLab::Json(Chat->GetFeed());else if(Feature==TEXT("Marketplace"))Text=SeagullLab::Json(Session()->Marketplace->GetCurrent(TEXT("state")));
    Status->SetText(FText::FromString(Text.IsEmpty()?TEXT("Read the current feature explicitly first."):Text.Left(12000)+TEXT("\n[Display preview only; the SDK snapshot and original are unchanged.]")));
}
void USeagullFeatureLab::Connect(){if(auto* GI=Cast<USeagullGameInstance>(GetGameInstance())){GI->GetHorizonManager()->ConnectToServer();Status->SetText(FText::FromString(TEXT("Explicit connection started; no signup. Restore or create a player separately.")));}}
void USeagullFeatureLab::Restore()
{
    if(auto* GI=Cast<USeagullGameInstance>(GetGameInstance()))
    {
        TWeakObjectPtr<USeagullFeatureLab> Weak(this);TWeakObjectPtr<USeagullGameInstance> Owner(GI);
        GI->GetHorizonManager()->RestoreSession([Weak,Owner](bool Ok)
        {
            if(Ok&&Owner.IsValid())Owner->BeginOnlineSession(false);
            if(auto* W=Weak.Get())W->Status->SetText(FText::FromString(Ok?TEXT("Existing player restored. Read current features explicitly."):TEXT("Restore failed; original identity retained, no replacement created.")));
        });
    }
}
void USeagullFeatureLab::CreateGuest()
{
    if(auto* GI=Cast<USeagullGameInstance>(GetGameInstance()))
    {
        auto* HM=GI->GetHorizonManager();
        if(HM->IsSignedIn()||HM->HasProtectedGuestIdentity()){Status->SetText(FText::FromString(TEXT("An existing/protected identity is present. Restore it explicitly; no new player was created.")));return;}
        TWeakObjectPtr<USeagullFeatureLab> Weak(this);TWeakObjectPtr<USeagullGameInstance> Owner(GI);
        HM->SignUpAnonymous(TEXT("SeagullHello"),[Weak,Owner](bool Ok)
        {
            if(Ok&&Owner.IsValid())Owner->BeginOnlineSession(true);
            if(auto* W=Weak.Get())W->Status->SetText(FText::FromString(Ok?TEXT("Player connected."):TEXT("Creation refused or uncertain. Reconcile protected identity before another signup.")));
        });
    }
}
void USeagullFeatureLab::MarketplaceFinished(bool Success,const FString& Code){Status->SetText(FText::FromString(Success?TEXT("Marketplace original confirmed and current Inventory/PlayerValues/Marketplace refreshed."):TEXT("Original retained: ")+Code));}
void USeagullFeatureLab::ClearRecipes(){for(AActor* Actor:{static_cast<AActor*>(Friends),static_cast<AActor*>(Groups),static_cast<AActor*>(Items),static_cast<AActor*>(Shared),static_cast<AActor*>(Matches),static_cast<AActor*>(Inbox),static_cast<AActor*>(Achievements),static_cast<AActor*>(Chat)})if(IsValid(Actor))Actor->Destroy();Friends=nullptr;Groups=nullptr;Items=nullptr;Shared=nullptr;Matches=nullptr;Inbox=nullptr;Achievements=nullptr;Chat=nullptr;Marketplace=nullptr;BoundMarketProject.Reset();}
void USeagullFeatureLab::NativeDestruct(){if(ChatPanel)ChatPanel->RemoveFromParent();ChatPanel=nullptr;ClearRecipes();Super::NativeDestruct();}
void USeagullFeatureLab::FocusControls(){FocusMenu();}
void USeagullFeatureLab::Close(){RemoveFromParent();OnClosed.ExecuteIfBound();}

void USeagullChatPanel::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    auto* Panel=SeagullWidgetStyles::MakePanel(WidgetTree,SeagullColors::PanelBG,24);WidgetTree->RootWidget=Panel;
    auto* Scroll=WidgetTree->ConstructWidget<UScrollBox>();Panel->SetContent(Scroll);
    auto* Column=WidgetTree->ConstructWidget<UVerticalBox>();Scroll->AddChild(Column);
    Body=CreateWidget<UHorizonChatExampleWidget>(GetOwningPlayer());
    if(Body)SeagullWidgetStyles::AddRow(Column,Body);
    auto* Back=SeagullWidgetStyles::MakeTextButton(WidgetTree,TEXT("Back to feature lab"),SeagullColors::Orange,SeagullColors::DarkText);
    Back->OnClicked.AddDynamic(this,&USeagullChatPanel::Close);SeagullWidgetStyles::AddRow(Column,Back);
    SetMenuBack(FSimpleDelegate::CreateUObject(this,&USeagullChatPanel::Close));
}
void USeagullChatPanel::BindExample(AHorizonChatExample* Example){if(Body)Body->Example=Example;}
void USeagullChatPanel::Close(){RemoveFromParent();}

void ASeagullFeaturesHelloGameMode::BeginPlay()
{
    Super::BeginPlay();
    if(auto* PC=UGameplayStatics::GetPlayerController(this,0))
    {
        if(auto* Lab=CreateWidget<USeagullFeatureLab>(PC)){Lab->AddToViewport(100);Lab->FocusMenu();}
    }
}
