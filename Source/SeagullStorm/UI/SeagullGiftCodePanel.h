#pragma once

#include "CoreMinimal.h"
#include "UI/SeagullMenu.h"
#include "SeagullGiftCodePanel.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;

UCLASS()
class USeagullGiftCodePanel : public USeagullMenu
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& Geometry,float DeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UEditableTextBox* CodeInput = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* RedeemButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* CloseButton = nullptr;

    UPROPERTY() UButton* ConfirmButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StatusText = nullptr;

private:
	bool bRedeeming = false;
    bool bObservedBusy = false;
	void CompleteRequest(const FString& Message);

	UFUNCTION()
	void OnRedeemClicked();

    UFUNCTION() void OnConfirmClicked();

	UFUNCTION()
	void OnCloseClicked();
};
