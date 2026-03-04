#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Data/SeagullSaveData.h"
#include "Models/HorizonNewsEntry.h"
#include "SeagullGameInstance.generated.h"

class USeagullConfigCache;
class USeagullHorizonManager;
class UHorizonSubsystem;

UCLASS()
class USeagullGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UHorizonSubsystem* GetHorizonSubsystem() const;
	USeagullHorizonManager* GetHorizonManager();
	USeagullConfigCache* GetConfigCache();

	UPROPERTY()
	FSeagullSaveData SaveData;

	bool bConfigLoaded = false;
	bool bSaveLoaded = false;

	bool bNewsLoaded = false;
	TArray<FHorizonNewsEntry> CachedNews;

	// Track consecutive wave-1 deaths for warn log
	int32 ConsecutiveWave1Deaths = 0;

private:
	UPROPERTY()
	USeagullHorizonManager* HorizonManager = nullptr;

	UPROPERTY()
	USeagullConfigCache* ConfigCache = nullptr;
};
