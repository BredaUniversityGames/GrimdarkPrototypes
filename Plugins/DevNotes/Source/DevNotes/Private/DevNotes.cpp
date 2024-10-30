// Copyright Epic Games, Inc. All Rights Reserved.

#include "DevNotes.h"

#include "BugData.h"
#include "BugReportButtonStyle.h"
#include "BugReportCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

#include "DevNoteActor.h"
#include "HttpModule.h"
#include "JiraBugList.h"
#include "Interfaces/IHttpResponse.h"

static const FName BugReportTabName("BugReport");

#define LOCTEXT_NAMESPACE "FDevNotesModule"

void FDevNotesModule::StartupModule()
{
	FBugReportButtonStyle::Initialize();
	FBugReportButtonStyle::ReloadTextures();

	FBugReportCommands::Register();

	BugReportCommands = MakeShareable(new FUICommandList);

	BugReportCommands->MapAction
	(
		FBugReportCommands::Get().BugReportAction,
		FExecuteAction::CreateRaw(this, &FDevNotesModule::BugReportButtonClicked),
		FCanExecuteAction()
	);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FDevNotesModule::RegisterMenus));
	GetAllJiraBugs(nullptr);
}

void FDevNotesModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FBugReportButtonStyle::Shutdown();

	FBugReportCommands::Unregister();
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FDevNotesModule::BugReportButtonClicked()
{
}

void FDevNotesModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("BugReportWindowSection");
			Section.AddSeparator(FName(TEXT("BugReportWindowSeparator")));
			Section.AddMenuEntryWithCommandList(FBugReportCommands::Get().BugReportAction, BugReportCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("BugReportToolbarSection");
			{
				Section.AddSeparator(FName(TEXT("BugReportToolbarSeparator")));
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FBugReportCommands::Get().BugReportAction));
				Entry.SetCommandList(BugReportCommands);
			}
		}
	}
}

bool FDevNotesModule::CreateJiraIssue(const FString& Name, const FString& Description, const FString& Priority)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	if(Name.Len() < 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bug name %s is too short"), *Name);
		return false;
	}

	if(Description.Len() < 20)
	{
		UE_LOG(LogTemp, Warning, TEXT("Description \n %s \n is too short"), *Description);
		return false;
	}

	FString BugDescription = Description;
	for(int i = 0; i < BugDescription.Len(); i++)
		if(BugDescription[i] == '\n')
		{
			BugDescription[i] = '\\';
			BugDescription.InsertAt(i + 1, "n");
			i++;
		}
		else if (BugDescription[i] == '\r')
		{
			BugDescription.RemoveAt(i);
			i--;
		}
	
	Request->SetURL(TEXT("https://jira.buas.nl/rest/api/2/issue"));
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	FString B = FString("Bearer ") + FString("PersonalToken");
	Request->SetHeader("Authorization", *B);
	
	FString JiraIssuePayload = FString::Printf(TEXT(R"(
    {
        "fields": {
            "project": { "key": "Y12223BEE" },
            "summary": "%s",
            "description": "%s",
            "issuetype": { "name": "Bug" },
			"priority": { "name": "%s" }
        }
    })"), *Name, *BugDescription, *Priority);

	Request->SetContentAsString(JiraIssuePayload);

	Request->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Request, FHttpResponsePtr Response, const bool bWasSuccessful)
	{
		if (bWasSuccessful && Response->GetResponseCode() == 201)
		{
			UE_LOG(LogTemp, Log, TEXT("Jira issue created successfully: %s"), *Response->GetContentAsString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create Jira issue. Response: %s"), *Response->GetContentAsString());
		}
	});

	Request->ProcessRequest();
	return true;
}

void FDevNotesModule::RequestJiraBugs(UJiraBugList* BugListWidget)
{
	GetAllJiraBugs(BugListWidget);
}

bool FDevNotesModule::GetAllJiraBugs(UJiraBugList* BugListWidget)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(TEXT("https://jira.buas.nl/rest/api/2/search"));
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Accept", "application/json");
	FString B = FString("Bearer ") + FString("PersonalToken");
	Request->SetHeader("Authorization", *B);
	FString JiraIssuePayload = FString::Printf(TEXT(R"(
    {
	    "expand": ["names", "schema"],
	    "fields": ["summary", "description", "status", "assignee", "priority"],
        "jql": "project = Y12223BEE AND issuetype = Bug",
        "maxResults": 100,
        "startAt": 0
    })"));

	Request->SetContentAsString(JiraIssuePayload);
	Request->OnProcessRequestComplete().BindRaw(this, &FDevNotesModule::OnGetJiraBugs, BugListWidget);

	// Execute the request
	Request->ProcessRequest();
	
	return true;
}

void FDevNotesModule::OnGetJiraBugs(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, UJiraBugList* BugListWidget)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get a valid response from Jira."));
		//BugListWidget->FillBugList(TArray<FBugData>());
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	const bool Success = FJsonSerializer::Deserialize(Reader, JsonObject);
	if (Success && JsonObject.IsValid())
	{
		TArray<FBugData> BugList;
		const TArray<TSharedPtr<FJsonValue>>* Issues;
		if (JsonObject->TryGetArrayField(TEXT("issues"), Issues))
		{
			for (const TSharedPtr<FJsonValue>& IssueValue : *Issues)
			{
				TSharedPtr<FJsonObject> IssueObject = IssueValue->AsObject();
				
				FString KeyS = "key";
				const FStringView Key(KeyS);
				FString IssueKey = IssueObject->GetStringField(Key);
				
				const TSharedPtr<FJsonObject>* Fields;
				if (IssueObject->TryGetObjectField(TEXT("fields"), Fields))
				{
					FString Summary = Fields->Get()->GetStringField(TEXT("summary"));
					FString Description = Fields->Get()->GetStringField(TEXT("description"));
					FString Priority = Fields->Get()->GetObjectField(TEXT("priority"))->GetStringField(TEXT("name"));
					FString Assignee = Fields->Get()->GetObjectField(TEXT("assignee"))->GetStringField(TEXT("name"));
					BugList.Emplace(IssueKey, Summary, Description, Priority, Assignee);
					//UE_LOG(LogTemp, Log, TEXT("Issue Key: %s, Summary: %s, Description : %s, Priority: %s"), *IssueKey, *Summary, *Description, *Priority);
				}
			}
		}
		//BugListWidget->FillBugList(BugList);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse Jira response as JSON."));
	}
	
}

void FDevNotesModule::CheckClickedActor(AActor* Actor)
{
	if(ADevNoteActor* DevNoteActor = Cast<ADevNoteActor>(Actor))
	{
		//DevNoteActor->CheckClickedActor();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDevNotesModule, DevNotes)