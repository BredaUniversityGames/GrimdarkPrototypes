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
#include "SimJumpCharacter.h"
#include "SimulationCharacterMovementComponent.h"
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
	//FCoreDelegates::OnEndFrame.AddRaw(this, &FJumpVisualizationModule::FindAndModifyJumpLocations);
	//FEngineShowFlags::RegisterCustomShowFlag()

	ViewFlagName = TEXT("JumpVisualization");
	ViewFlagIndex = static_cast<uint32>(FEngineShowFlags::FindIndexByName(*ViewFlagName));
}

void FJumpVisualizationModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FJumpVisualizationModule::CalculateJumpLocation(const TArray<TArray<FCapsuleLocation>>& SessionJumps, TArray<TArray<FCapsuleLocation>>& OutputJumps, const AJumpVisActor* VisActor)
{
	OutputJumps.Empty();
	if(SessionJumps.Num() == 0)
		return;
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if(!World)
		World = GEditor->PlayWorld;

	if(!World)
		return;
	
	//TSharedPtr<USimulationCharacterMovementComponent> SimChMovComp = MakeShared<USimulationCharacterMovementComponent>();
	//SimCh = World->SpawnActor<ASimJumpCharacter>();
	//TSharedPtr<ASimJumpCharacter> SimCh = MakeShared<ASimJumpCharacter>();
	for(TActorIterator<ASimJumpCharacter> ActorIt(World); ActorIt; ++ActorIt)
	{
		SimCh = *ActorIt;
	}
	if(!SimCh)
		return;
	USimulationCharacterMovementComponent* SimChMovComp = Cast<USimulationCharacterMovementComponent>(SimCh->GetMovementComponent());
	if(!SimChMovComp)
		return;

	if(World->bPostTickComponentUpdate)
		return;
	AirControlJumpRange.Empty();
	for(int j = 0; j < SessionJumps.Num(); j++)
	{
		SimChMovComp->SetCharacterOwner(SimCh);
		SimChMovComp->SetUpdatedComponent(SimCh->GetCapsuleComponent());
		//TArray<FCapsuleLocation> JumpLocations;
		FVector Velocity2D = SessionJumps[j][0].Velocity;
		Velocity2D.Z = 0.f;
		FVector Direction = Velocity2D.GetSafeNormal();
		SimChMovComp->Velocity = Direction * VisActor->Speed;
		SimChMovComp->Velocity.Z = VisActor->JumpZVelocity;
		SimChMovComp->AirControl = SessionJumps[j][0].AirControl;
		SimChMovComp->AirControlBoostMultiplier = SessionJumps[j][0].AirControlBoostMultiplier;
		SimChMovComp->AirControlBoostVelocityThreshold = SessionJumps[j][0].AirControlBoostVelocityThreshold;
		SimChMovComp->MaxSimulationIterations = SessionJumps[j][0].MaxSimulationIterations;
		SimChMovComp->MaxSimulationTimeStep = SessionJumps[j][0].MaxSimulationTimeStep;
		SimCh->SetActorLocation(SessionJumps[j][0].Location);
		SimCh->SetActorRotation(SessionJumps[j][0].Rotation);
		if(VisActor->UseDifferentAirControl)
		{
			SimChMovComp->AirControl = VisActor->AirControl;
		}
		//TArray<TArray<FCapsuleLocation>> Output;
		OutputJumps.Emplace(TArray<FCapsuleLocation>());
		float Angle = VisActor->AirControlDirection * 90.f;
		FRotator Rotation(0.f, Angle, 0.f);
		FVector RotatedDirection = Rotation.RotateVector(Direction);
		FVector VisActorAcc = RotatedDirection * SessionJumps[j][0].MaxAcceleration;
		for(int i = 0; i < 720 || i < SessionJumps[j].Num(); i++)
		{
			if(SessionJumps[j].Num() > i)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Acceleration: %s"), *VisActorAcc.ToString());
				if(VisActor->UseDifferentAirControl)
					SimChMovComp->SetAcceleration(VisActorAcc);
				else
					SimChMovComp->SetAcceleration(SessionJumps[j][i].Acceleration);
			}
			
			SimChMovComp->PhysFalling(0.016f, 0);
			FCapsuleLocation Location;
			Location.TopMiddle = SimChMovComp->GetActorLocation();
			Location.BottomMiddle = SimChMovComp->GetActorLocation();
			Location.TopMiddle.Z += SessionJumps[j][0].HalfCapsuleHeight;
			Location.BottomMiddle.Z -= SessionJumps[j][0].HalfCapsuleHeight;
			Location.Location = SimChMovComp->GetActorLocation();
			OutputJumps.Last().Emplace(Location);
		}
		ShowAirControlRange = VisActor->ShowAirControlRange;
		if(VisActor->ShowAirControlRange)
		{
			AirControlJumpRange.Emplace(TPair<TArray<FCapsuleLocation>, TArray<FCapsuleLocation>>(TArray<FCapsuleLocation>(), TArray<FCapsuleLocation>()));
			SimCh->SetActorLocation(SessionJumps[j][0].Location);
			SimCh->SetActorRotation(SessionJumps[j][0].Rotation);
			SimChMovComp->Velocity = Direction * VisActor->Speed;
			SimChMovComp->Velocity.Z = VisActor->JumpZVelocity;
			float LeftAngle = -90.f;
			FRotator LeftRotation(0.f, LeftAngle, 0.f);
			FVector LeftRotatedDirection = LeftRotation.RotateVector(Direction);
			FVector LeftVisActorAcc = LeftRotatedDirection * SessionJumps[j][0].MaxAcceleration;
			for(int i = 0; i < 720 || i < SessionJumps[j].Num(); i++)
			{
				if(SessionJumps[j].Num() > i)
				{
					SimChMovComp->SetAcceleration(LeftVisActorAcc);
				}
			
				SimChMovComp->PhysFalling(0.016f, 0);
				FCapsuleLocation Location;
				Location.TopMiddle = SimChMovComp->GetActorLocation();
				Location.BottomMiddle = SimChMovComp->GetActorLocation();
				Location.TopMiddle.Z += SessionJumps[j][0].HalfCapsuleHeight;
				Location.BottomMiddle.Z -= SessionJumps[j][0].HalfCapsuleHeight;
				Location.Location = SimChMovComp->GetActorLocation();
				AirControlJumpRange.Last().Key.Emplace(Location);
			}
			
			//AirControlJumpRange.Emplace(TPair<TArray<FCapsuleLocation>, TArray<FCapsuleLocation>>(TArray<FCapsuleLocation>(), TArray<FCapsuleLocation>()));
			SimCh->SetActorLocation(SessionJumps[j][0].Location);
			SimCh->SetActorRotation(SessionJumps[j][0].Rotation);
			SimChMovComp->Velocity = Direction * VisActor->Speed;
			SimChMovComp->Velocity.Z = VisActor->JumpZVelocity;
			float RightAngle = 90.f;
			FRotator RightRotation(0.f, RightAngle, 0.f);
			FVector RightRotatedDirection = RightRotation.RotateVector(Direction);
			FVector RightVisActorAcc = RightRotatedDirection * SessionJumps[j][0].MaxAcceleration;
			for(int i = 0; i < 720 || i < SessionJumps[j].Num(); i++)
			{
				if(SessionJumps[j].Num() > i)
				{
					SimChMovComp->SetAcceleration(RightVisActorAcc);
				}
			
				SimChMovComp->PhysFalling(0.016f, 0);
				FCapsuleLocation Location;
				Location.TopMiddle = SimChMovComp->GetActorLocation();
				Location.BottomMiddle = SimChMovComp->GetActorLocation();
				Location.TopMiddle.Z += SessionJumps[j][0].HalfCapsuleHeight;
				Location.BottomMiddle.Z -= SessionJumps[j][0].HalfCapsuleHeight;
				Location.Location = SimChMovComp->GetActorLocation();
				AirControlJumpRange.Last().Value.Emplace(Location);
			}
		}
	}
}

void FJumpVisualizationModule::FindAndModifyJumpLocations(const AJumpVisActor* VisActor)
{
	TArray<uint8> BinaryData;
	bool bFoundFile = false;
	//FJumpVisualizationModule& JumpVisualizationModule = FModuleManager::GetModuleChecked<FJumpVisualizationModule>("JumpVisualization");
	TArray<TArray<FCapsuleLocation>> CurrentJumpLocations;
	FString FilePath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/"), GetNFile(bFoundFile, GetSessionNumberToShow()));
	if(FFileHelper::LoadFileToArray(BinaryData, *FilePath) && bFoundFile)
	{
		FMemoryReader Archive(BinaryData, true);
		Archive.Seek(0);

		Archive << CurrentJumpLocations;
	}
	
	//for(TActorIterator<AJumpVisActor> ActorIt(GEditor->PlayWorld); ActorIt; ++ActorIt)
	//{
	//	JumpVisActor = *ActorIt;
	//}
//
	//if(!JumpVisActor)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("Could not find jump visualization actor in world. Make sure there is one!"));
	//	return;
	//}
	ModifiedJumpLocations.Empty();
	if(VisActor->UseSetValues)
		CalculateJumpLocation(CurrentJumpLocations, ModifiedJumpLocations, VisActor);
	else ModifiedJumpLocations = CurrentJumpLocations;
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
	GEditor->GetTimerManager()->SetTimer(CollectResourceDataTimer, TimerDelegate, 0.016f, true, 0.f);

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
		GEditor->GetTimerManager()->SetTimer(CollectJumpDataTimer, TimerDelegate, 0.016f, true, 0.f);
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
	Location.Acceleration = MovementRef->GetCurrentAcceleration();
	Location.MaxAcceleration = MovementRef->GetMaxAcceleration();
	Location.MaxSimulationIterations = MovementRef->MaxSimulationIterations;
	Location.MaxSimulationTimeStep = MovementRef->MaxSimulationTimeStep;
	Location.AirControl = MovementRef->AirControl;
	Location.AirControlBoostMultiplier = MovementRef->AirControlBoostMultiplier;
	Location.AirControlBoostVelocityThreshold = MovementRef->AirControlBoostVelocityThreshold;
	Location.JumpZVelocity = MovementRef->JumpZVelocity;
	Location.Velocity = MovementRef->Velocity;
	Location.GravityScale = MovementRef->GravityScale;
	Location.Location = CapsuleRef->GetComponentLocation();
	Location.HalfCapsuleHeight = HalfHeight;
	Location.TopMiddle = Location.BottomMiddle = CapsuleRef->GetComponentLocation();
	Location.TopMiddle.Z += HalfHeight;
	Location.BottomMiddle.Z -= HalfHeight;
	Location.Time = FDateTime::Now();
	Location.Velocity = Character->GetVelocity();
	Location.Speed = Character->GetVelocity().Size();
	Location.GravityScale = MovementRef->GravityScale;
	Location.JumpZVelocity = MovementRef->JumpZVelocity;
	
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