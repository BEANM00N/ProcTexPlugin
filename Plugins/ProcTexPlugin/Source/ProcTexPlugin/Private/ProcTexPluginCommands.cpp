// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProcTexPluginCommands.h"

#define LOCTEXT_NAMESPACE "FProcTexPluginModule"

void FProcTexPluginCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "ProcTexPlugin", "Open ProcTex", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
