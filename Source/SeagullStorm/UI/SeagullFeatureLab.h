#pragma once
#include "CoreMinimal.h"
#include "UI/SeagullMenu.h"
#include "Components/ComboBoxString.h"
#include "GameFramework/SaveGame.h"
#include "GameFramework/GameModeBase.h"
#include "SeagullFeatureLab.generated.h"
class UComboBoxString;class UEditableTextBox;class UMultiLineEditableTextBox;class UTextBlock;class UHorizonSubsystem;
class AHorizonFriendsExample;class AHorizonGroupsExample;class AHorizonItemsExample;class AHorizonSharedObjectsExample;class AHorizonMatchesExample;class AHorizonInboxExample;class AHorizonAchievementsExample;class AHorizonChatExample;class UHorizonMarketplaceExample;
class UHorizonChatExampleWidget;

/** Navigation host only; the official Chat widget and its actions stay unchanged. */
UCLASS()
class USeagullChatPanel : public USeagullMenu
{
    GENERATED_BODY()
public:
    virtual void NativeOnInitialized() override;
    void BindExample(AHorizonChatExample* Example);
private:
    UPROPERTY() UHorizonChatExampleWidget* Body = nullptr;
    UFUNCTION() void Close();
};
UCLASS()
class USeagullProjectBinding:public USaveGame
{
    GENERATED_BODY()
public: UPROPERTY() FString Project;UPROPERTY() FString KeyHash;
};
/** Game and standalone Hello share explicit existing-session actions. The official SDK owns original journals. */
UCLASS()
class USeagullFeatureLab:public USeagullMenu
{
    GENERATED_BODY()
public:
    virtual void NativeOnInitialized()override;
    virtual void NativeDestruct()override;
    void FocusControls();
    FSimpleDelegate OnClosed;
    /** Fixed action dispatch also used by native consumer acceptance. No arbitrary method invocation. */
    bool Execute(const FString& Feature,const FString& Action,const FString& Arguments,FString& Error);
private:
    UPROPERTY() UComboBoxString* FeatureInput=nullptr;
    UPROPERTY() UComboBoxString* ActionInput=nullptr;
    UPROPERTY() UEditableTextBox* ProjectInput=nullptr;
    UPROPERTY() UMultiLineEditableTextBox* ArgumentsInput=nullptr;
    UPROPERTY() UTextBlock* Status=nullptr;
    UPROPERTY() AHorizonFriendsExample* Friends=nullptr;
    UPROPERTY() AHorizonGroupsExample* Groups=nullptr;
    UPROPERTY() AHorizonItemsExample* Items=nullptr;
    UPROPERTY() AHorizonSharedObjectsExample* Shared=nullptr;
    UPROPERTY() AHorizonMatchesExample* Matches=nullptr;
    UPROPERTY() AHorizonInboxExample* Inbox=nullptr;
    UPROPERTY() AHorizonAchievementsExample* Achievements=nullptr;
    UPROPERTY() AHorizonChatExample* Chat=nullptr;
    UPROPERTY() USeagullChatPanel* ChatPanel=nullptr;
    UPROPERTY() UHorizonMarketplaceExample* Marketplace=nullptr;
    FString BoundMarketProject;uint64 LastContext=MAX_uint64;
    UFUNCTION() void SelectFeature(FString Selected,ESelectInfo::Type Selection);
    UFUNCTION() void RunSelected();
    UFUNCTION() void ShowSnapshot();
    UFUNCTION() void ReadPlayerValues();
    UFUNCTION() void ReadStatistics();
    UFUNCTION() void Connect();
    UFUNCTION() void Restore();
    UFUNCTION() void CreateGuest();
    UFUNCTION() void Close();
    UFUNCTION() void MarketplaceFinished(bool Success,const FString& Code);
    UHorizonSubsystem* Session()const;
    bool BindMarketplace(FString& Error);
    void ClearRecipes();
};

/** Launch /Game/Maps/MainMap?game=/Script/SeagullStorm.SeagullFeaturesHelloGameMode for a standalone Hello. */
UCLASS()
class ASeagullFeaturesHelloGameMode:public AGameModeBase
{
    GENERATED_BODY()
public: virtual void BeginPlay()override;
};
