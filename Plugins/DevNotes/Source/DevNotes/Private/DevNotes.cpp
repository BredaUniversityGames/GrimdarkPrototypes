// Copyright Epic Games, Inc. All Rights Reserved.

#include "DevNotes.h"

#include "ActorPickerMode.h"
#include "DevNoteActor.h"

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

void FDevNotesModule::CheckClickedActor(AActor* Actor)
{
	if(ADevNoteActor* DevNoteActor = Cast<ADevNoteActor>(Actor))
	{
		//DevNoteActor->CheckClickedActor();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDevNotesModule, DevNotes)