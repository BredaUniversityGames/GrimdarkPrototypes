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
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "JumpVisData.h"

#define LOCTEXT_NAMESPACE "FJumpVisualizationModule"

void FJumpVisualizationModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//FCustomEditorViewportCommands::Register();
	//BindCommands();
	
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FJumpVisualizationModule::RegisterMenuExtensions));
	FEditorDelegates::EndPIE.AddRaw(this, &FJumpVisualizationModule::PrintJumpLocations);
	
	if(GEngine)
	{
		DrawDebugJumpsDelegate = UDebugDrawService::Register(TEXT("Editor"), FDebugDrawDelegate::CreateRaw(this, &FJumpVisualizationModule::OnDrawJumpDebug));
	}
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
		RecordJumpsDelegate = FEditorDelegates::PostPIEStarted.AddRaw(this, &FJumpVisualizationModule::StartRecordingJumps);
	}
	else
	{
		FEditorDelegates::PostPIEStarted.Remove(RecordJumpsDelegate);
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
	
	//TActorIterator<UJumpVisData> ActorItr(Character->GetWorld());
	
	JumpLocations.push_back(Location);
	if(!MovementRef->IsFalling())
	{
		GEditor->GetTimerManager()->ClearTimer(CollectJumpDataTimer);
		CheckJumpDelegate = FCoreDelegates::OnEndFrame.AddRaw(this, &FJumpVisualizationModule::DidCharacterJustJump);
	}
}

void FJumpVisualizationModule::PrintJumpLocations(bool IsSimulating)
{
	for(CapsuleLocation& Location : JumpLocations)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bottom: %s,   Top: %s \n"), *Location.BottomMiddle.ToString(), *Location.TopMiddle.ToString());
	}
	FCoreDelegates::OnEndFrame.Remove(CheckJumpDelegate);
	JumpLocations.clear();
}

void FJumpVisualizationModule::OnDrawJumpDebug(UCanvas* Canvas, APlayerController* PC)
{
	if(Canvas)
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
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FJumpVisualizationModule, JumpVisualization)