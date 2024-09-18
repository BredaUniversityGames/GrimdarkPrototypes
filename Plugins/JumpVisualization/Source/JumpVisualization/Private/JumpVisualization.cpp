// Copyright Epic Games, Inc. All Rights Reserved.

#include "JumpVisualization.h"

#include "DebugRenderSceneProxy.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "FCustomEditorViewportCommands.h"
#include "JumpVisActor.h"
#include "ToolMenus.h"
#include "Components/CapsuleComponent.h"
#include "Debug/DebugDrawService.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Serialization/BufferArchive.h"
#include "JumpVisActor.h"
#include "Containers/DirectoryTree.h"

#define LOCTEXT_NAMESPACE "FJumpVisualizationModule"

void FJumpVisualizationModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//FCustomEditorViewportCommands::Register();
	//BindCommands();
	
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FJumpVisualizationModule::RegisterMenuExtensions));
	FEditorDelegates::EndPIE.AddRaw(this, &FJumpVisualizationModule::PrintJumpLocations);
	RecordJumpsDelegate = FEditorDelegates::PostPIEStarted.AddRaw(this, &FJumpVisualizationModule::StartRecordingJumps);
	RecordJumpsDelegate = FEditorDelegates::ShutdownPIE.AddRaw(this, &FJumpVisualizationModule::OnEndPIE);
	
	//FEngineShowFlags::RegisterCustomShowFlag()
}

void FJumpVisualizationModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FJumpVisualizationModule::ToggleJumpVisualization()
{
	IsJumpVisible = !IsJumpVisible;
	if(IsJumpVisible)
	{
		PrintJumpLocations(false);
		
	}
	else
	{
		FlushPersistentDebugLines(GEditor->GetEditorWorldContext().World());
		//FEditorDelegates::PostPIEStarted.Remove(RecordJumpsDelegate);
	}
	//PawnRef->
}

bool FJumpVisualizationModule::IsJumpVisualizationVisible() const
{
	return IsJumpVisible;
}

void FJumpVisualizationModule::BindCommands()
{
	const FCustomEditorViewportCommands& Commands = FCustomEditorViewportCommands::Get();
	CommandList = MakeShared<FUICommandList>();
	CommandList->MapAction(
	  	Commands.JumpVisualization,
	 	FExecuteAction::CreateRaw(this, &FJumpVisualizationModule::ToggleJumpVisualization),
	  	FCanExecuteAction(),
	  	FIsActionChecked::CreateRaw(this, &FJumpVisualizationModule::IsJumpVisualizationVisible));
}

void FJumpVisualizationModule::RegisterMenuExtensions()
{
	//BindCommands();
	FToolMenuOwnerScoped OwnerScoped(this);
	UToolMenu* ShowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelViewportToolbar.Show");
	FToolMenuSection& ShowMenuSection = ShowMenu->FindOrAddSection("CustomLevelViewportEditorShow");
	ShowMenuSection.AddSeparator(FName("Custom Level Editor Tools"));
	FUIAction Action (FExecuteAction::CreateRaw(this, &FJumpVisualizationModule::ToggleJumpVisualization),
		  FCanExecuteAction(),
		  FIsActionChecked::CreateRaw(this, &FJumpVisualizationModule::IsJumpVisualizationVisible));
	ShowMenuSection.AddMenuEntry(FName("JumpVisualizer"), FText::FromString("Jump Visualizer"), FText::FromString("Visualize Jumps From Previous Play"), FSlateIcon(), Action, EUserInterfaceActionType::ToggleButton);
}

void FJumpVisualizationModule::StartRecordingJumps(bool IsSimulating)
{
	JumpLocations.Empty();
	for(TActorIterator<AJumpVisActor> ActorIt(GEditor->PlayWorld); ActorIt; ++ActorIt)
	{
		JumpVisActor = *ActorIt;
	}

	if(!JumpVisActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find jump visualization actor in world. Make sure there is one!"));
		return;
	}
	JumpVisActor->JumpLocations.Empty();
	CheckJumpDelegate = FCoreDelegates::OnEndFrame.AddRaw(this, &FJumpVisualizationModule::DidCharacterJustJump);
}

void FJumpVisualizationModule::DidCharacterJustJump()
{
	APlayerController* PlayerControllerRef = UGameplayStatics::GetPlayerController(GEditor->PlayWorld, 0);
	if(!PlayerControllerRef)
		return;
	APawn* PawnRef = PlayerControllerRef->GetPawn();
	ACharacter* CharacterRef = Cast<ACharacter>(PawnRef);
	if(!CharacterRef)
		return;
	
	if(CharacterRef->bWasJumping)
	{
		FCoreDelegates::OnEndFrame.Remove(CheckJumpDelegate);
		JumpLocations.Add(TArray<FCapsuleLocation>());
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateRaw(this, &FJumpVisualizationModule::CollectJumpData, CharacterRef);
		GEditor->GetTimerManager()->SetTimer(CollectJumpDataTimer, TimerDelegate, 0.1f, true, 0.f);
	}
}

void FJumpVisualizationModule::CollectJumpData(ACharacter* Character)
{
	if(!Character)
		return;
	UCharacterMovementComponent* MovementRef = Character->GetCharacterMovement();
	UCapsuleComponent* CapsuleRef = Character->GetCapsuleComponent();
	if(!MovementRef || !CapsuleRef)
		return;
	FCapsuleLocation Location;
	float Radius = 0.f;
	float HalfHeight = 0.f;
	CapsuleRef->GetScaledCapsuleSize(Radius, HalfHeight);
	Location.TopMiddle = Location.BottomMiddle = CapsuleRef->GetComponentLocation();
	Location.TopMiddle.Z += HalfHeight;
	Location.BottomMiddle.Z -= HalfHeight;
	Location.Time = FDateTime::Now();
	
	JumpLocations.Last().Add(Location);
	if(MovementRef && !MovementRef->IsFalling())
	{
		GEditor->GetTimerManager()->ClearTimer(CollectJumpDataTimer);
		CheckJumpDelegate = FCoreDelegates::OnEndFrame.AddRaw(this, &FJumpVisualizationModule::DidCharacterJustJump);
	}
}

void FJumpVisualizationModule::PrintJumpLocations(bool IsSimulating)
{
	FCoreDelegates::OnEndFrame.Remove(CheckJumpDelegate);
	GEditor->GetTimerManager()->ClearTimer(CollectJumpDataTimer);
	if(!JumpVisActor)
	{
		return;
	}
	
	JumpVisActor->JumpLocations = JumpLocations;
}

void FJumpVisualizationModule::OnEndPIE(bool IsSimulating)
{
	FBufferArchive Archive;
	Archive << JumpLocations;
	FDateTime CurrentTime = FDateTime::Now();
	FString NewFileName = TEXT("JumpDataFile");
	NewFileName.Append(CurrentTime.ToString());
	NewFileName.Append(TEXT(".dat"));
	FString FullPath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/"), NewFileName);
	TArray<FString> FileNames;
	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	while(FileNames.Num() > 99)
	{
		FString OldestFile = GetOldestFile();
		IFileManager::Get().Delete(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/"), OldestFile), false, true);
	}
	
	if(FFileHelper::SaveArrayToFile(Archive, *FullPath))
	{
		Archive.FlushCache();
		Archive.Empty();
	}
}

FString FJumpVisualizationModule::GetOldestFile()
{
	TArray<FString> FileNames;
	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	FString OldestFile = TEXT("");
	FDateTime OldestFileTimestamp = FDateTime::MaxValue();
	for(int i = 0; i < FileNames.Num(); i++)
	{
		//18
		FString FileName = FileNames[i];
		FileName.RemoveFromEnd(".dat");
		FString TimeStampString = FileName.RightChop(18);

		FDateTime Timestamp;
		FDateTime::Parse(TimeStampString, Timestamp);

		if(Timestamp < OldestFileTimestamp)
		{
			OldestFileTimestamp = Timestamp;
			OldestFile = FileNames[i];
		}
	}
	return OldestFile;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FJumpVisualizationModule, JumpVisualization)