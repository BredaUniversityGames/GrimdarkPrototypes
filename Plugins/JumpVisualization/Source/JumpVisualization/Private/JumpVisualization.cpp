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
#include "LevelEditor.h"
#include "SLevelViewport.h"
#include "Containers/DirectoryTree.h"
#include "Framework/MultiBox/SToolBarComboButtonBlock.h"

#define LOCTEXT_NAMESPACE "FJumpVisualizationModule"

void FJumpVisualizationModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//FCustomEditorViewportCommands::Register();
	//BindCommands();
	
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FJumpVisualizationModule::RegisterMenuExtensions));
	FEditorDelegates::EndPIE.AddRaw(this, &FJumpVisualizationModule::PrintJumpLocations);
	RecordJumpsDelegate = FEditorDelegates::PostPIEStarted.AddRaw(this, &FJumpVisualizationModule::StartRecordingData);
	RecordJumpsDelegate = FEditorDelegates::ShutdownPIE.AddRaw(this, &FJumpVisualizationModule::OnEndPIE);
	
	//FEngineShowFlags::RegisterCustomShowFlag()

	ViewFlagName = TEXT("JumpVisualization");
	ViewFlagIndex = static_cast<uint32>(FEngineShowFlags::FindIndexByName(*ViewFlagName));
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
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
		if (LevelEditor.IsValid())
		{
			TArray<TSharedPtr<SLevelViewport>> Viewports = LevelEditor->GetViewports();
			for (const TSharedPtr<SLevelViewport>& ViewportWindow : Viewports)
			{
				if (ViewportWindow.IsValid())
				{
					FEditorViewportClient& Viewport = ViewportWindow->GetAssetViewportClient();
					Viewport.EngineShowFlags.SetSingleFlag(ViewFlagIndex, true);
				}
			}
		}
	}
	else
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
		if (LevelEditor.IsValid())
		{
			TArray<TSharedPtr<SLevelViewport>> Viewports = LevelEditor->GetViewports();
			for (const TSharedPtr<SLevelViewport>& ViewportWindow : Viewports)
			{
				if (ViewportWindow.IsValid())
				{
					FEditorViewportClient& Viewport = ViewportWindow->GetAssetViewportClient();
					Viewport.EngineShowFlags.SetSingleFlag(ViewFlagIndex, false);
				}
			}
		}
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
	TSharedPtr<SComboButton> ComboButton = SNew(SComboButton).AccessibleText(LOCTEXT("Combo", ""));
	
	//ShowMenuSection.AddEntry(FToolMenuEntry::InitWidget("ComboButton", ComboButton.ToSharedRef(), FText::GetEmpty()));
	//ShowMenuSection.AddSubMenu()
	//ShowMenuSection.AddMenuEntry(FName("JumpVisualizer"), FText::FromString("Jump Visualizer"), FText::FromString("Visualize Jumps From Previous Play"), FSlateIcon(), Action, EUserInterfaceActionType::);
}

void FJumpVisualizationModule::StartRecordingData(bool IsSimulating)
{
	JumpLocations.Empty();
	for(auto& Pair : ResourceData)
		Pair.Value.Empty();
	for(TActorIterator<AJumpVisActor> ActorIt(GEditor->PlayWorld); ActorIt; ++ActorIt)
	{
		JumpVisActor = *ActorIt;
	}

	if(!JumpVisActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find jump visualization actor in world. Make sure there is one!"));
		return;
	}

	CheckJumpDelegate = FCoreDelegates::OnEndFrame.AddRaw(this, &FJumpVisualizationModule::CheckPlayerData);
	//CheckResourceDelegate = FCoreDelegates::OnEndFrame.AddRaw(this, &FJumpVisualizationModule::CollectResourceData);
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateRaw(this, &FJumpVisualizationModule::CollectResourceData);
	GEditor->GetTimerManager()->SetTimer(CollectResourceDataTimer, TimerDelegate, 0.5f, true, 0.f);

}

void FJumpVisualizationModule::CheckPlayerData()
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
		CheckJumpDelegate = FCoreDelegates::OnEndFrame.AddRaw(this, &FJumpVisualizationModule::CheckPlayerData);
	}
}

void FJumpVisualizationModule::CollectResourceData()
{
	APlayerController* PlayerControllerRef = UGameplayStatics::GetPlayerController(GEditor->PlayWorld, 0);
	if(!PlayerControllerRef)
		return;
	APawn* PawnRef = PlayerControllerRef->GetPawn();
	ACharacter* CharacterRef = Cast<ACharacter>(PawnRef);
	if(!CharacterRef)
		return;
	UCharacterMovementComponent* MovementRef = CharacterRef->GetCharacterMovement();
	UCapsuleComponent* CapsuleRef = CharacterRef->GetCapsuleComponent();
	if(!MovementRef || !CapsuleRef)
		return;
	float Radius = 0.f;
	float HalfHeight = 0.f;
	CapsuleRef->GetScaledCapsuleSize(Radius, HalfHeight);
	FVector Location =  CapsuleRef->GetComponentLocation();
	FDateTime Time = FDateTime::Now();
	for(auto& Resource : ResourceData)
	{
		Resource.Value.Emplace(FResourceData(*Resource.Key, Location, Time));
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
		bool bFoundFile = false;
		FString OldestFile = GetOldestFile(bFoundFile);
		if(!bFoundFile)
		{
			break;
		}
		IFileManager::Get().Delete(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/"), OldestFile), false, true);
	}
	
	if(FFileHelper::SaveArrayToFile(Archive, *FullPath))
	{
		Archive.FlushCache();
		Archive.Empty();
	}
}

FString FJumpVisualizationModule::GetOldestFile(bool& bFoundFile)
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
			bFoundFile = true;
		}
	}
	return OldestFile;
}

FString FJumpVisualizationModule::GetNewestFile(bool& bFoundFile)
{
	TArray<FString> FileNames;
	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	FString NewestFile = TEXT("");
	FDateTime NewestFileTimestamp = FDateTime::MinValue();
	for(int i = 0; i < FileNames.Num(); i++)
	{
		//18
		FString FileName = FileNames[i];
		FileName.RemoveFromEnd(".dat");
		FileName.RightInline(19);
		
		FDateTime Timestamp;
		FDateTime::Parse(FileName, Timestamp);

		if(Timestamp > NewestFileTimestamp)
		{
			NewestFileTimestamp = Timestamp;
			NewestFile = FileNames[i];
			bFoundFile = true;
		}
	}
	return NewestFile;
}

FString FJumpVisualizationModule::GetNFile(bool& bFoundFile, int N)
{
	TArray<FString> FileNames;
	TMap<FDateTime, FString> FileMap;
	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	FString NewestFile = TEXT("");
	for(int i = 0; i < FileNames.Num(); i++)
	{
		//18
		FString FileName = FileNames[i];
		FileName.RemoveFromEnd(".dat");
		FileName.RightInline(19);
		
		FDateTime Timestamp;
		FDateTime::Parse(FileName, Timestamp);

		FileMap.Add(Timestamp, FileNames[i]);
	}
	
	TArray<FDateTime> Keys;
	FileMap.GenerateKeyArray(Keys);
	N = Keys.Num() - N;
	for(int i = Keys.Num() - 1; i >= 0; i--)
	{
		if(i == N)
		{
			bFoundFile = true;
        	return FileMap[Keys[i]];
		}
	}
	
	bFoundFile = false;
	return "";
}

int FJumpVisualizationModule::GetAmountOfFiles()
{	
	TArray<FString> FileNames;
 	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	return FileNames.Num();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FJumpVisualizationModule, JumpVisualization)