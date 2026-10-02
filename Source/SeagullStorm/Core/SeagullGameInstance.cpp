#include "Core/SeagullGameInstance.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Data/SeagullConfigCache.h"
#include "HorizonSubsystem.h"
#include "SeagullStorm.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonSerializer.h"

void USeagullGameInstance::Init()
{
	Super::Init();
	SaveData.InitDefaults();
	UE_LOG(LogSeagullStorm, Log, TEXT("SeagullGameInstance initialized"));
}

UHorizonSubsystem* USeagullGameInstance::GetHorizonSubsystem() const
{
	return GetSubsystem<UHorizonSubsystem>();
}

USeagullHorizonManager* USeagullGameInstance::GetHorizonManager()
{
	if (!HorizonManager)
	{
		HorizonManager = NewObject<USeagullHorizonManager>(this);
		HorizonManager->Initialize(GetHorizonSubsystem());
	}
	return HorizonManager;
}

USeagullConfigCache* USeagullGameInstance::GetConfigCache()
{
	if (!ConfigCache)
	{
		ConfigCache = NewObject<USeagullConfigCache>(this);
		ConfigCache->ParseFromConfigs({}); // Populate offline arrays, stats and upgrade cards.
	}
	return ConfigCache;
}

void USeagullGameInstance::BeginPractice()
{
    ++SessionGeneration;
    GetHorizonManager()->DiscardValidatedRun();
    bPracticeMode = true;
    bSaveLoaded = true;
    bCloudSaveReady = false;
    SaveOwnerId = TEXT("practice");
    SaveData.InitDefaults();
    FString Json;
    if (FFileHelper::LoadFileToString(Json, *(FPaths::ProjectSavedDir() / TEXT("SeagullStorm/practice.json"))))
    {
        TSharedPtr<FJsonObject> Object;
        if (FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Object) && Object.IsValid())
            SaveData = FSeagullSaveData::FromJsonString(Json);
    }
    GetConfigCache()->ParseFromConfigs({}); // Practice always uses local defaults.
	bConfigLoaded = false;
}

void USeagullGameInstance::BeginOnlineSession(bool bKnownNewPlayer)
{
    const FString UserId = GetHorizonManager()->GetUserId();
    if (bPracticeMode || SaveOwnerId != UserId)
    {
        ++SessionGeneration;
        GetHorizonManager()->DiscardValidatedRun();
        SaveData.InitDefaults();
        bSaveLoaded = false;
        bCloudSaveReady = false;
		GetConfigCache()->ParseFromConfigs({});
		bConfigLoaded = false;
		bNewsLoaded = false;
		CachedNews.Empty();
    }
    bPracticeMode = false;
    SaveOwnerId = UserId;
	if (bKnownNewPlayer) { bSaveLoaded = true; bCloudSaveReady = true; }
}

bool USeagullGameInstance::CanMutateProgress()
{
    USeagullHorizonManager* HM = GetHorizonManager();
    return bPracticeMode || (bSaveLoaded && bCloudSaveReady && HM && HM->IsSignedIn()
        && !SaveOwnerId.IsEmpty() && SaveOwnerId == HM->GetUserId());
}

void USeagullGameInstance::PersistPracticeSave()
{
    if (!bPracticeMode) return;
    const FString Directory = FPaths::ProjectSavedDir() / TEXT("SeagullStorm");
    IFileManager::Get().MakeDirectory(*Directory, true);
    FFileHelper::SaveStringToFile(SaveData.ToJsonString(), *(Directory / TEXT("practice.json")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}
