#pragma once

#include "CoreMinimal.h"
#include "UI/SeagullMenu.h"
#include "SeagullSettingsPanel.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class USeagullSettingsPanel : public USeagullMenu
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* SignOutButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* CloseButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* UserInfoText = nullptr;

private:
	UFUNCTION()
	void OnSignOutClicked();

	UFUNCTION()
	void OnCloseClicked();
};
