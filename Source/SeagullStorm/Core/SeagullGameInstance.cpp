#include "Core/SeagullGameInstance.h"
#include "Horizon/SeagullHorizonManager.h"
#include "Data/SeagullConfigCache.h"
#include "HorizonSubsystem.h"
#include "SeagullStorm.h"

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
	}
	return ConfigCache;
}
