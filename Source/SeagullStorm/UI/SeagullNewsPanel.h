#pragma once

#include "CoreMinimal.h"
#include "UI/SeagullMenu.h"
#include "SeagullNewsPanel.generated.h"

class UTextBlock;
class UButton;
class UVerticalBox;

UCLASS()
class USeagullNewsPanel : public USeagullMenu
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	void LoadNews();

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UVerticalBox* NewsList = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* CloseButton = nullptr;

private:
	UFUNCTION()
	void OnCloseClicked();
};
