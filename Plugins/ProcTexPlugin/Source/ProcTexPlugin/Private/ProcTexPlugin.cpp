// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProcTexPlugin.h"
#include "ProcTexPluginStyle.h"
#include "ProcTexPluginCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"

// --- NEW: Includes required to launch Editor Utility Widgets ---
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"

static const FName ProcTexPluginTabName("ProcTexPlugin");

#define LOCTEXT_NAMESPACE "FProcTexPluginModule"

void FProcTexPluginModule::StartupModule()
{
    FProcTexPluginStyle::Initialize();
    FProcTexPluginStyle::ReloadTextures();

    FProcTexPluginCommands::Register();
    
    PluginCommands = MakeShareable(new FUICommandList);

    PluginCommands->MapAction(
       FProcTexPluginCommands::Get().PluginAction,
       FExecuteAction::CreateRaw(this, &FProcTexPluginModule::PluginButtonClicked),
       FCanExecuteAction());

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FProcTexPluginModule::RegisterMenus));
}

void FProcTexPluginModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
    FProcTexPluginStyle::Shutdown();
    FProcTexPluginCommands::Unregister();
}

void FProcTexPluginModule::PluginButtonClicked()
{
	FString WidgetPath = TEXT("/ProcTexPlugin/Utilities/TexturePipeline/TextureGenerator.TextureGenerator");
	UObject* BlueprintObject = StaticLoadObject(UObject::StaticClass(), nullptr, *WidgetPath);
    
	if (UEditorUtilityWidgetBlueprint* UtilityBlueprint = Cast<UEditorUtilityWidgetBlueprint>(BlueprintObject))
	{
		if (UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
		{
			// Spawn the tab and save a reference to the created widget
			UEditorUtilityWidget* SpawnedWidget = EditorUtilitySubsystem->SpawnAndRegisterTab(UtilityBlueprint);
            
			if (SpawnedWidget)
			{
				// Force the Slate widget to initialize its layout in memory
				SpawnedWidget->ForceLayoutPrepass();
                
				// Get the underlying C++ Slate widget
				TSharedPtr<SWidget> SlateWidget = SpawnedWidget->GetCachedWidget();
				if (SlateWidget.IsValid())
				{
					// Find the physical Editor window hosting this tab and maximize it!
					TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(SlateWidget.ToSharedRef());
					if (ParentWindow.IsValid())
					{
						ParentWindow->Maximize();
					}
				}
			}
		}
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Could not find EUW.")));
	}
}

void FProcTexPluginModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);

    {
       UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
       {
          FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
          Section.AddMenuEntryWithCommandList(FProcTexPluginCommands::Get().PluginAction, PluginCommands);
       }
    }

    {
       UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
       {
          FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
          {
             FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FProcTexPluginCommands::Get().PluginAction));
             Entry.SetCommandList(PluginCommands);
          }
       }
    }
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FProcTexPluginModule, ProcTexPlugin)