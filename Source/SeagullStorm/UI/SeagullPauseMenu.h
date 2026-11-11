#pragma once

#include "CoreMinimal.h"
#include "UI/SeagullMenu.h"
#include "SeagullPauseMenu.generated.h"

class UButton;

UCLASS()
class USeagullPauseMenu : public USeagullMenu
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* ResumeButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* NewsButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* FeedbackButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* QuitButton = nullptr;

private:
	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnNewsClicked();

	UFUNCTION()
	void OnFeedbackClicked();

	UFUNCTION()
	void OnQuitClicked();
};
