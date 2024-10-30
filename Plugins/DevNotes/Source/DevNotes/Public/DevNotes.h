// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Modules/ModuleManager.h"

class UJiraBugList;

class FDevNotesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void BugReportButtonClicked();

	bool CreateJiraIssue(const FString& Name, const FString& Description, const FString& Priority);
	void RequestJiraBugs(UJiraBugList* BugListWidget);
	
protected:
	void RegisterMenus();
	bool GetAllJiraBugs(UJiraBugList* BugListWidget);
	void OnGetJiraBugs(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, UJiraBugList* BugListWidget);
	void CheckClickedActor(AActor* Actor);

public:

protected:
	TSharedPtr<FUICommandList> BugReportCommands;
};
