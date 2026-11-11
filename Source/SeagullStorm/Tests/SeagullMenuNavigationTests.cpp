#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/SeagullMenu.h"
#include "UI/SeagullWidgetStyles.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/VerticalBox.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/Input/SButton.h"

// Uses actual UMG/Slate routing, with no game facade, SDK session, storage or HTTP.
struct FSeagullMenuNavigationFixture final : IAutomationLatentCommand
{
    FAutomationTestBase* Test;
    TWeakObjectPtr<USeagullMenu> Parent, Modal;
    TWeakObjectPtr<UEditableTextBox> Input;
    TWeakObjectPtr<UButton> Confirm, Last;
    double Started=FPlatformTime::Seconds();
    int32 Step=0, Confirmed=0;
    explicit FSeagullMenuNavigationFixture(FAutomationTestBase* InTest):Test(InTest){}
    static void Key(FKey Key, bool Shift=false)
    {
        const FModifierKeysState Modifiers(Shift,false,false,false,false,false,false,false,false);
        FSlateApplication::Get().ProcessKeyDownEvent(FKeyEvent(Key,Modifiers,0,false,0,0));
        FSlateApplication::Get().ProcessKeyUpEvent(FKeyEvent(Key,Modifiers,0,false,0,0));
    }
    virtual bool Update() override
    {
        if(FPlatformTime::Seconds()-Started>10)
        {Test->AddError(TEXT("Offline menu navigation did not complete"));if(Modal.IsValid())Modal->RemoveFromParent();if(Parent.IsValid())Parent->RemoveFromParent();return true;}
        if(Step==0)
        {
            UWorld* World=nullptr;for(const auto& Context:GEngine->GetWorldContexts())if(Context.World()&&Context.World()->IsGameWorld()){World=Context.World();break;}
            auto* PC=World?World->GetFirstPlayerController():nullptr;if(!PC)return false;
            auto* Menu=CreateWidget<USeagullMenu>(PC);Parent=Menu;
            auto* Column=Menu->WidgetTree->ConstructWidget<UVerticalBox>();Menu->WidgetTree->RootWidget=Column;
            Input=SeagullWidgetStyles::MakeTextInput(Menu->WidgetTree,TEXT("Offline editing"));Column->AddChild(Input.Get());
            Confirm=SeagullWidgetStyles::MakeTextButton(Menu->WidgetTree,TEXT("Confirm"),FLinearColor::Blue,FLinearColor::White);Column->AddChild(Confirm.Get());
            auto* Hidden=SeagullWidgetStyles::MakeTextButton(Menu->WidgetTree,TEXT("Hidden"),FLinearColor::Blue,FLinearColor::White);Hidden->SetVisibility(ESlateVisibility::Collapsed);Column->AddChild(Hidden);
            Last=SeagullWidgetStyles::MakeTextButton(Menu->WidgetTree,TEXT("Last"),FLinearColor::Blue,FLinearColor::White);Column->AddChild(Last.Get());
            Menu->AddToViewport(500);
            StaticCastSharedPtr<SButton>(Confirm->GetCachedWidget())->SetOnClicked(FOnClicked::CreateLambda([this](){++Confirmed;return FReply::Handled();}));
            ++Step;return false;
        }
        if(Step==1)
        {
            Test->TestEqual(TEXT("First editable control has real Slate focus"),Parent->FocusedControl(),static_cast<UWidget*>(Input.Get()));
            Key(EKeys::Enter);Key(EKeys::SpaceBar);
            Test->TestEqual(TEXT("Editing Enter/Space never confirms a button"),Confirmed,0);
            Key(EKeys::Tab);Test->TestEqual(TEXT("Tab moves to confirm"),Parent->FocusedControl(),static_cast<UWidget*>(Confirm.Get()));
            Key(EKeys::Tab,true);Test->TestEqual(TEXT("ShiftTab returns to input"),Parent->FocusedControl(),static_cast<UWidget*>(Input.Get()));
            Key(EKeys::Tab);Key(EKeys::Enter);Test->TestEqual(TEXT("Enter confirms only focused button"),Confirmed,1);
            Key(EKeys::Down);Test->TestEqual(TEXT("Arrow skips hidden control"),Parent->FocusedControl(),static_cast<UWidget*>(Last.Get()));
            Key(EKeys::Gamepad_DPad_Up);Test->TestEqual(TEXT("DPad returns to confirm"),Parent->FocusedControl(),static_cast<UWidget*>(Confirm.Get()));
            Key(EKeys::Gamepad_FaceButton_Bottom);Test->TestEqual(TEXT("South confirms focused button"),Confirmed,2);
            auto* Child=CreateWidget<USeagullMenu>(Parent->GetOwningPlayer());Modal=Child;
            auto* Column=Child->WidgetTree->ConstructWidget<UVerticalBox>();Child->WidgetTree->RootWidget=Column;
            Column->AddChild(SeagullWidgetStyles::MakeTextButton(Child->WidgetTree,TEXT("Back"),FLinearColor::Blue,FLinearColor::White));
            Child->SetMenuBack(FSimpleDelegate::CreateLambda([this](){if(Modal.IsValid())Modal->RemoveFromParent();}));
            Child->AddToViewport(501);++Step;return false;
        }
        if(Step==2)
        {
            Test->TestFalse(TEXT("Underlying menu is disabled while modal owns focus"),Parent->GetIsEnabled());
            Key(EKeys::Tab);Test->TestTrue(TEXT("Tab wraps inside the top modal"),Modal->FocusedControl()!=nullptr);
            Key(EKeys::Gamepad_FaceButton_Right);++Step;return false;
        }
        Test->TestFalse(TEXT("East removes the modal through its explicit Back action"),Modal->IsInViewport());
        Test->TestTrue(TEXT("Parent is enabled again"),Parent->GetIsEnabled());
        Test->TestEqual(TEXT("Closing restores the actual parent control"),Parent->FocusedControl(),static_cast<UWidget*>(Confirm.Get()));
        Key(EKeys::Tab,true);Key(EKeys::Tab,true);Test->TestEqual(TEXT("ShiftTab wraps within the parent"),Parent->FocusedControl(),static_cast<UWidget*>(Last.Get()));
        Parent->RemoveFromParent();return true;
    }
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSeagullMenuNavigation,"Seagull.UI.KeyboardGamepadNavigation",EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)
bool FSeagullMenuNavigation::RunTest(const FString&){ADD_LATENT_AUTOMATION_COMMAND(FSeagullMenuNavigationFixture(this));return true;}
#endif
