#include "UI/SeagullMenu.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Slider.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Rendering/DrawElements.h"

TArray<TWeakObjectPtr<USeagullMenu>> USeagullMenu::OpenMenus;

namespace
{
void CollectControls(UWidget* Widget, TArray<TWeakObjectPtr<UWidget>>& Out)
{
    if (!Widget || !Widget->IsVisible() || !Widget->GetIsEnabled()) return;
    if (Cast<UButton>(Widget) || Cast<UEditableTextBox>(Widget) || Cast<UMultiLineEditableTextBox>(Widget)
        || Cast<UComboBoxString>(Widget) || Cast<USlider>(Widget) || Cast<UCheckBox>(Widget))
    {
        if (Widget->GetCachedWidget().IsValid() && Widget->GetCachedWidget()->SupportsKeyboardFocus()) Out.Add(Widget);
        return;
    }
    // The Chat panel embeds the unchanged official SDK widget.
    if (auto* UserWidget = Cast<UUserWidget>(Widget))
    {
        if (UserWidget->WidgetTree) CollectControls(UserWidget->WidgetTree->RootWidget, Out);
    }
    else if (auto* Panel = Cast<UPanelWidget>(Widget))
        for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index) CollectControls(Panel->GetChildAt(Index), Out);
}
}

bool USeagullMenu::IsTopMenu() const
{
    for (int32 Index = OpenMenus.Num() - 1; Index >= 0; --Index)
        if (const auto* Menu = OpenMenus[Index].Get())
            if (Menu->GetOwningPlayer() == GetOwningPlayer()) return Menu == this;
    return false;
}

void USeagullMenu::NativeConstruct()
{
    Super::NativeConstruct();
    OpenMenus.RemoveAll([](const auto& Menu) { return !Menu.IsValid(); });
    for (int32 Index = OpenMenus.Num() - 1; Index >= 0; --Index)
        if (auto* Menu = OpenMenus[Index].Get())
            if (Menu->GetOwningPlayer() == GetOwningPlayer())
            {
                if (auto* Control = Menu->FocusedControl()) Menu->LastFocus = Control;
                Menu->SetIsEnabled(false);
                break;
            }
    OpenMenus.Add(this);
    SetIsEnabled(true);
    RefreshNavigation();
    FocusMenu();
}

void USeagullMenu::NativeDestruct()
{
    OpenMenus.Remove(this);
    for (int32 Index = OpenMenus.Num() - 1; Index >= 0; --Index)
        if (auto* Menu = OpenMenus[Index].Get())
            if (Menu->GetOwningPlayer() == GetOwningPlayer())
            {
                Menu->SetIsEnabled(true);
                Menu->FocusMenu();
                break;
            }
    Super::NativeDestruct();
}

void USeagullMenu::RefreshNavigation()
{
    TArray<TWeakObjectPtr<UWidget>> Current;
    if (WidgetTree) CollectControls(WidgetTree->RootWidget, Current);
    if (Current == Controls) return;
    Controls = MoveTemp(Current);
    for (int32 Index = 0; Index < Controls.Num(); ++Index)
    {
        auto* Control = Controls[Index].Get();
        auto* Previous = Controls[(Index + Controls.Num() - 1) % Controls.Num()].Get();
        auto* Next = Controls[(Index + 1) % Controls.Num()].Get();
        for (auto Direction : {EUINavigation::Previous, EUINavigation::Up, EUINavigation::Left}) Control->SetNavigationRuleExplicit(Direction, Previous);
        for (auto Direction : {EUINavigation::Next, EUINavigation::Down, EUINavigation::Right}) Control->SetNavigationRuleExplicit(Direction, Next);
        if (auto* Combo = Cast<UComboBoxString>(Control)) Combo->SetEnableGamepadNavigationMode(true);
        if (auto* Text = Cast<UEditableTextBox>(Control)) Text->SetClearKeyboardFocusOnCommit(false);
    }
}

UWidget* USeagullMenu::FocusedControl() const
{
    for (const auto& Entry : Controls)
        if (auto* Control = Entry.Get())
            if (Control->HasKeyboardFocus() || Control->HasFocusedDescendants()) return Control;
    return nullptr;
}

bool USeagullMenu::HasOpenCombo() const
{
    for (const auto& Entry : Controls)
        if (auto* Combo = Cast<UComboBoxString>(Entry.Get())) if (Combo->IsOpen()) return true;
    return false;
}

void USeagullMenu::FocusControl(UWidget* Control)
{
    if (!Control) return;
    LastFocus = Control;
    Control->SetKeyboardFocus();
    // Keep the focused input visible when a menu is taller than the viewport.
    if (WidgetTree) WidgetTree->ForEachWidget([Control](UWidget* Widget)
    {
        if (auto* Scroll = Cast<UScrollBox>(Widget)) Scroll->ScrollWidgetIntoView(Control, true, EDescendantScrollDestination::IntoView);
    });
}

void USeagullMenu::FocusMenu()
{
    if (!IsTopMenu()) return;
    RefreshNavigation();
    auto* Target = Controls.Contains(LastFocus) ? LastFocus.Get() : Controls.Contains(FirstFocus) ? FirstFocus.Get() : Controls.IsEmpty() ? nullptr : Controls[0].Get();
    if (auto* PC = GetOwningPlayer())
    {
        PC->bShowMouseCursor = true;
        FInputModeUIOnly Mode;
        if (Target) Mode.SetWidgetToFocus(Target->TakeWidget());
        PC->SetInputMode(Mode);
    }
    FocusControl(Target);
}

void USeagullMenu::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);
    if (!IsTopMenu()) return;
    RefreshNavigation();
    if (auto* Control = FocusedControl()) LastFocus = Control;
    // A combo popup temporarily owns a separate Slate focus path. Do not steal it.
    else if (!HasOpenCombo()) FocusMenu();
}

FReply USeagullMenu::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
    if (!IsTopMenu()) return FReply::Unhandled();
    if (HasOpenCombo()) return Super::NativeOnPreviewKeyDown(Geometry, Event);
    RefreshNavigation();
    if (Event.GetKey() == EKeys::Tab)
    {
        const int32 Current = Controls.IndexOfByKey(FocusedControl());
        if (!Controls.IsEmpty())
        {
            const int32 Next = Current < 0 ? (Event.IsShiftDown() ? Controls.Num() - 1 : 0)
                : (Current + (Event.IsShiftDown() ? Controls.Num() - 1 : 1)) % Controls.Num();
            FocusControl(Controls[Next].Get());
        }
        return FReply::Handled();
    }
    if (Event.GetKey() == EKeys::Escape || Event.GetKey() == EKeys::Gamepad_FaceButton_Right)
    {
        if (!Event.IsRepeat()) BackAction.ExecuteIfBound();
        return FReply::Handled();
    }
    // Slate owns arrows/DPad/stick and Confirm on the actual focused control.
    // In particular, Enter/Space in a text box never forwards to another button.
    return Super::NativeOnPreviewKeyDown(Geometry, Event);
}

int32 USeagullMenu::NativePaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& CullingRect,
    FSlateWindowElementList& Elements, int32 Layer, const FWidgetStyle& Style, bool ParentEnabled) const
{
    const int32 Painted = Super::NativePaint(Args, Geometry, CullingRect, Elements, Layer, Style, ParentEnabled);
    if (IsTopMenu()) if (const auto* Control = FocusedControl())
    {
        // Tick geometry includes the desktop window offset; paint geometry uses
        // the same window-local space as this paint pass.
        const auto& FocusGeometry = Control->GetPaintSpaceGeometry();
        const FVector2D Position = Geometry.AbsoluteToLocal(FocusGeometry.GetAbsolutePosition());
        const FVector2D Size = Geometry.AbsoluteToLocal(FocusGeometry.GetAbsolutePosition() + FocusGeometry.GetAbsoluteSize()) - Position;
        const FVector2D TopLeft = Position + FVector2D(1.5f, 1.5f);
        const FVector2D BottomRight = Position + Size - FVector2D(1.5f, 1.5f);
        const TArray<FVector2D> Outline = {TopLeft, FVector2D(BottomRight.X, TopLeft.Y), BottomRight,
            FVector2D(TopLeft.X, BottomRight.Y), TopLeft};
        FSlateDrawElement::MakeLines(Elements, Painted + 1, Geometry.ToPaintGeometry(), Outline,
            ESlateDrawEffect::None, FLinearColor::White, true, 3.f);
        return Painted + 1;
    }
    return Painted;
}
