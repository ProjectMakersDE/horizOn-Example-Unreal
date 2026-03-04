#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SeagullGiftCodePanel.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;

UCLASS()
class USeagullGiftCodePanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UEditableTextBox* CodeInput = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* RedeemButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* CloseButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StatusText = nullptr;

private:
	UFUNCTION()
	void OnRedeemClicked();

	UFUNCTION()
	void OnCloseClicked();
};
