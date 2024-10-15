// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDevNotesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void BugReportButtonClicked();

private:
	void RegisterMenus();
	void CreateJiraIssue();
	void CheckClickedActor(AActor* Actor);

private:
	TSharedPtr<FUICommandList> BugReportCommands;
};
