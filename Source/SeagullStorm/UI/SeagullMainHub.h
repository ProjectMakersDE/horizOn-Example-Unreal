#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SeagullMainHub.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class USeagullMainHub : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* PlayButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* GiftCodeButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* FeedbackButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* SettingsButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* CoinsText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* HighscoreText = nullptr;

private:
	UFUNCTION()
	void OnPlayClicked();

	UFUNCTION()
	void OnGiftCodeClicked();

	UFUNCTION()
	void OnFeedbackClicked();

	UFUNCTION()
	void OnSettingsClicked();

	void LoadHubData();
	void RefreshDisplay();

	// Upgrade buttons
	UFUNCTION()
	void OnBuySpeed();
	UFUNCTION()
	void OnBuyDamage();
	UFUNCTION()
	void OnBuyHP();
	UFUNCTION()
	void OnBuyMagnet();

	void TryBuyUpgrade(const FString& Key);
};
