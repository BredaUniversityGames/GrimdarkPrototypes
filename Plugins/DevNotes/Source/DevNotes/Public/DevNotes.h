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

	bool CreateJiraIssue(const FString& Name, const FString& Description, const FString& Priority);
private:
	void RegisterMenus();
	void CheckClickedActor(AActor* Actor);

private:
	TSharedPtr<FUICommandList> BugReportCommands;
};
