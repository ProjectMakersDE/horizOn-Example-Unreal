#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SeagullMenu.generated.h"

/** Local UMG focus/navigation shared by the example's menus, not its SDK or requests. */
UCLASS()
class USeagullMenu : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
    virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& CullingRect,
        FSlateWindowElementList& Elements, int32 Layer, const FWidgetStyle& Style, bool ParentEnabled) const override;

    void FocusMenu();
protected:
    void SetMenuFirstFocus(UWidget* Control) { FirstFocus = Control; }
    void SetMenuBack(FSimpleDelegate Action) { BackAction = MoveTemp(Action); }
private:
    friend struct FSeagullMenuNavigationFixture;
    static TArray<TWeakObjectPtr<USeagullMenu>> OpenMenus;
    TArray<TWeakObjectPtr<UWidget>> Controls;
    TWeakObjectPtr<UWidget> FirstFocus;
    TWeakObjectPtr<UWidget> LastFocus;
    FSimpleDelegate BackAction;
    bool IsTopMenu() const;
    bool HasOpenCombo() const;
    void RefreshNavigation();
    void FocusControl(UWidget* Control);
    UWidget* FocusedControl() const;
};
