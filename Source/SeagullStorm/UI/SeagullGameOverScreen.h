#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SeagullGameOverScreen.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class USeagullGameOverScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ScoreText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* WavesText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* LevelText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* CoinsText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* RankText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* BestText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* PlayAgainButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* HubButton = nullptr;

private:
	UFUNCTION()
	void OnPlayAgainClicked();

	UFUNCTION()
	void OnHubClicked();

	void LoadGameOverData();
};
