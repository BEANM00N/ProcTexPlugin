// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "ProcTexPluginStyle.h"

class FProcTexPluginCommands : public TCommands<FProcTexPluginCommands>
{
public:

	FProcTexPluginCommands()
		: TCommands<FProcTexPluginCommands>(TEXT("ProcTexPlugin"), NSLOCTEXT("Contexts", "ProcTexPlugin", "ProcTexPlugin Plugin"), NAME_None, FProcTexPluginStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
