// Copyright Epic Games, Inc. All Rights Reserved.

#include "DevNotes.h"

#include "ActorPickerMode.h"
#include "DevNoteActor.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "PacketHandlers/StatelessConnectHandlerComponent.h"

#define LOCTEXT_NAMESPACE "FDevNotesModule"

void FDevNotesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//FOnActorSelected::BindRaw(this, &FDevNotesModule::CheckClickedActor);
}

void FDevNotesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FDevNotesModule::CreateJiraIssue()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(TEXT("https://jira.buas.nl/rest/api/2/issue"));
	Request->SetVerb("POST");
	//Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "application/json");
	FString B = FString("Bearer ") + FString("PersonalToken");
	Request->SetHeader("Authorization", *B);
	
	FString JiraIssuePayload = R"(
    {
        "fields": {
            "project": { "key": "Y12223BEE" },
            "summary": "Issue created from Unreal Engine",
            "description": "This is a test issue created from within Unreal Engine.",
            "issuetype": { "name": "Bug" }
        }
    })";

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