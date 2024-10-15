// Fill out your copyright notice in the Description page of Project Settings.


#include "BugReportLibrary.h"

#include "DevNotes.h"

bool UBugReportLibrary::CreateJiraBug(const FString& BugName, const FString& BugDescription, const FString& Priority)
{
	FDevNotesModule& DevNotesModule = FModuleManager::GetModuleChecked<FDevNotesModule>("DevNotes");
	return DevNotesModule.CreateJiraIssue(BugName, BugDescription, Priority);
}
