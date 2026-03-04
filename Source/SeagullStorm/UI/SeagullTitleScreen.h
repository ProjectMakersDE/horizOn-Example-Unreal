#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SeagullTitleScreen.generated.h"

class UButton;
class UTextBlock;
class UEditableTextBox;

UCLASS()
class USeagullTitleScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TitleText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SubtitleText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UEditableTextBox* NameInput = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* GuestButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* GoogleButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* EmailButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* RegisterButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StatusText = nullptr;

private:
	UFUNCTION()
	void OnGuestClicked();

	UFUNCTION()
	void OnGoogleClicked();

	UFUNCTION()
	void OnEmailClicked();

	UFUNCTION()
	void OnRegisterClicked();

	void OnAuthSuccess();
	void OnAuthFailure();
};
