// Fill out your copyright notice in the Description page of Project Settings.


#include "BugReportCommands.h"

#define LOCTEXT_NAMESPACE "FBugReportModule"

void FBugReportCommands::RegisterCommands()
{
	UI_COMMAND(BugReportAction, "BugReport", "Execute bug report button action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE