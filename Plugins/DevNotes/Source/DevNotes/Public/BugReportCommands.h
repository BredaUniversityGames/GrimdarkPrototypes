// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BugReportButtonStyle.h"
#include "Framework/Commands/Commands.h"

/**
 * 
 */
class DEVNOTES_API FBugReportCommands : public TCommands<FBugReportCommands>
{
public:
	FBugReportCommands()
		: TCommands<FBugReportCommands>(TEXT("BugReport"), NSLOCTEXT("Contexts", "Bug Report", "Bug Report Plugin"), NAME_None, FBugReportButtonStyle::GetStyleSetName())
	{
		
	}
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> BugReportAction;
};
