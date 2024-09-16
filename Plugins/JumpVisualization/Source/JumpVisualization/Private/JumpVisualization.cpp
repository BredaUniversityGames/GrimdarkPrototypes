// Copyright Epic Games, Inc. All Rights Reserved.

#include "JumpVisualization.h"

#include "DebugRenderSceneProxy.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "FCustomEditorViewportCommands.h"
#include "ToolMenus.h"
#include "Components/CapsuleComponent.h"
#include "Debug/DebugDrawService.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/Core/NetBitArray.h"

#define LOCTEXT_NAMESPACE "FJumpVisualizationModule"

void FJumpVisualizationModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//FCustomEditorViewportCommands::Register();
	//BindCommands();
	
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FJumpVisualizationModule::RegisterMenuExtensions));
	FEditorDelegates::EndPIE.AddRaw(this, &FJumpVisualizationModule::PrintJumpLocations);
	RecordJumpsDelegate = FEditorDelegates::PostPIEStarted.AddRaw(this, &FJumpVisualizationModule::StartRecordingJumps);
	if(GEngine)
	{
		DrawDebugJumpsDelegate = UDebugDrawService::Register(TEXT("Editor"), FDebugDrawDelegate::CreateRaw(this, &FJumpVisualizationModule::OnDrawJumpDebug));
	}
	
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
	//const FCustomEditorViewportCommands& Commands = FCustomEditorViewportCommands::Get();
	//ShowMenuSection.AddMenuEntry(FName("Jump Visualization"), )
	//ShowMenuSection.AddMenuEntry(FCustomEditorViewportCommands::Get().JumpVisualization);
	FUIAction Action (FExecuteAction::CreateRaw(this, &FJumpVisualizationModule::ToggleJumpVisualization),
		  FCanExecuteAction(),
		  FIsActionChecked::CreateRaw(this, &FJumpVisualizationModule::IsJumpVisualizationVisible));
	ShowMenuSection.AddMenuEntry(FName("JumpVisualizer"), FText::FromString("Jump Visualizer"), FText::FromString("Visualize Jumps From Previous Play"), FSlateIcon(), Action, EUserInterfaceActionType::ToggleButton);
}

void FJumpVisualizationModule::StartRecordingJumps(bool IsSimulating)
{
	JumpLocations.clear();
	FlushPersistentDebugLines(GEditor->GetEditorWorldContext().World());
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
	CapsuleLocation Location;
	float Radius = 0.f;
	float HalfHeight = 0.f;
	CapsuleRef->GetScaledCapsuleSize(Radius, HalfHeight);
	Location.TopMiddle = Location.BottomMiddle = CapsuleRef->GetComponentLocation();
	Location.TopMiddle.Z += HalfHeight;
	Location.BottomMiddle.Z -= HalfHeight;
	
	//JumpLocations.push_back(Location);
	
	JumpLocations.push_back(Location);
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
	//for(CapsuleLocation& Location : JumpLocations)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Bottom: %s,   Top: %s \n"), *Location.BottomMiddle.ToString(), *Location.TopMiddle.ToString());
	//}
	//FCoreDelegates::OnEndFrame.Remove(CheckJumpDelegate);
	//JumpLocations.clear();
	if(!IsJumpVisible)
		return;
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if(World && JumpLocations.size() >= 2)
	{
		for(int i = 0; i < JumpLocations.size() - 1; i++)
		{
			FVector Bottom1 = JumpLocations[i].BottomMiddle;
			FVector Top1 = JumpLocations[i].TopMiddle;
	
			FVector Bottom2 = JumpLocations[i + 1].BottomMiddle;
			FVector Top2 = JumpLocations[i + 1].TopMiddle;
			
			FLinearColor LineColor = FLinearColor::Red;
			FLinearColor LineColor2 = FLinearColor::Green;
			float LineThickness = 3.0f;
		
			DrawDebugLine(World, Bottom1, Bottom2, LineColor2.ToFColor(true), true, -1.0f, 0, LineThickness);
			DrawDebugLine(World, Top1, Top2, LineColor2.ToFColor(true), true, -1.0f, 0, LineThickness);
			DrawDebugLine(World, Bottom1, Top1, LineColor.ToFColor(true), true, -1.0f, 0, LineThickness);
			DrawDebugLine(World, Bottom2, Top2, LineColor.ToFColor(true), true, -1.0f, 0, LineThickness);
		}
	}
}

void FJumpVisualizationModule::OnDrawJumpDebug(UCanvas* Canvas, APlayerController* PC)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if(World)
	{
		FVector Start = FVector(0, 0, 0);
		FVector End = FVector(0, 0, 1000);
			
		FLinearColor LineColor = FLinearColor::Red;
		float LineThickness = 5.0f;
		DrawDebugLine(World, Start, End, LineColor.ToFColor(true), false, -1.0f, 0, LineThickness);
	}
	/*if(Canvas)
	{
		UWorld* World = PC ? PC->GetWorld() : nullptr;
		if (World)
		{
			FVector Start = FVector(0, 0, 0);
			FVector End = FVector(0, 0, 100);
			
			FLinearColor LineColor = FLinearColor::Green;
			float LineThickness = 2.0f;

			DrawDebugLine(World, Start, End, LineColor.ToFColor(true), false, -1.0f, 0, LineThickness);
		}
	}*/
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FJumpVisualizationModule, JumpVisualization)