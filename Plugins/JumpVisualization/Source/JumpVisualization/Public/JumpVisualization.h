// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FJumpVisualizationModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void ToggleJumpVisualization();
	bool IsJumpVisualizationVisible() const;
	void BindCommands();
	void RegisterMenuExtensions();

public:
	
private:
	bool IsJumpVisible = false;
	TSharedPtr<FUICommandList> CommandList;
};
