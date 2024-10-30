// Copyright Epic Games, Inc. All Rights Reserved.

#include "JumpVisualization.h"

#include "Editor.h"
#include "EngineUtils.h"
#include "JumpVisActor.h"
#include "ToolMenus.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Serialization/BufferArchive.h"
#include "LevelEditor.h"
#include "PlayerJumpData.h"
#include "SimJumpCharacter.h"
#include "SimulationCharacterMovementComponent.h"
#include "SLevelViewport.h"

#define LOCTEXT_NAMESPACE "FJumpVisualizationModule"

void FJumpVisualizationModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FJumpVisualizationModule::RegisterMenuExtensions));
	RecordJumpsDelegate = FEditorDelegates::PostPIEStarted.AddRaw(this, &FJumpVisualizationModule::StartRecordingData);
	RecordJumpsDelegate = FEditorDelegates::ShutdownPIE.AddRaw(this, &FJumpVisualizationModule::OnEndPIE);

	ViewFlagName = TEXT("JumpVisualization");
	ViewFlagIndex = static_cast<uint32>(FEngineShowFlags::FindIndexByName(*ViewFlagName));
}

void FJumpVisualizationModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FJumpVisualizationModule::CalculateJumpLocation(const TArray<TArray<FPlayerJumpData>>& SessionJumps, TArray<TArray<FPlayerJumpData>>& OutputJumps, const AJumpVisActor* VisActor)
{
	OutputJumps.Empty();
	if(SessionJumps.Num() == 0)
		return;
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if(!World)
		World = GEditor->PlayWorld;

	if(!World)
		return;
	
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
		if(VisActor->UseDifferentAirControl)
		{
			SimChMovComp->AirControl = VisActor->AirControl;
		}
		
		float Angle = VisActor->AirControlDirection * 90.f;
		OutputJumps.Add(EditSessionJumpData(SimChMovComp, SessionJumps[j], Angle, Direction, VisActor->UseDifferentAirControl));
		
		ShowAirControlRange = VisActor->ShowAirControlRange;
		if(ShowAirControlRange)
		{
			AirControlJumpRange.Emplace(TPair<TArray<FPlayerJumpData>, TArray<FPlayerJumpData>>(TArray<FPlayerJumpData>(), TArray<FPlayerJumpData>()));

			SimChMovComp->Velocity = Direction * VisActor->Speed;
			SimChMovComp->Velocity.Z = VisActor->JumpZVelocity;
			AirControlJumpRange.Last().Key = EditSessionJumpData(SimChMovComp, SessionJumps[j], -90.f, Direction, true);

			SimChMovComp->Velocity = Direction * VisActor->Speed;
			SimChMovComp->Velocity.Z = VisActor->JumpZVelocity;
			AirControlJumpRange.Last().Value = EditSessionJumpData(SimChMovComp, SessionJumps[j], 90.f, Direction, true);
		}
	}
}

void FJumpVisualizationModule::FindAndModifyJumpLocations(const AJumpVisActor* VisActor)
{
	FString FileName = "";
	const bool bFoundFile = GetNFile(GetSessionNumberToShow(), FileName);
	const FString FilePath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/"), FileName);

	TArray<TArray<FPlayerJumpData>> CurrentJumpLocations;
	TArray<uint8> BinaryData;
	if(FFileHelper::LoadFileToArray(BinaryData, *FilePath) && bFoundFile)
	{
		FMemoryReader Archive(BinaryData, true);
		Archive.Seek(0);
		Archive << CurrentJumpLocations;
	}

	ModifiedJumpLocations.Empty();
	if(VisActor->UseSetValues)
		CalculateJumpLocation(CurrentJumpLocations, ModifiedJumpLocations, VisActor);
	else
		ModifiedJumpLocations = CurrentJumpLocations;
}

void FJumpVisualizationModule::ToggleJumpVisualization()
{
	IsJumpVisible = !IsJumpVisible;
	const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	const TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
	if (LevelEditor.IsValid())
	{
		const TArray<TSharedPtr<SLevelViewport>> Viewports = LevelEditor->GetViewports();
		for (const TSharedPtr<SLevelViewport>& ViewportWindow : Viewports)
		{
			if (ViewportWindow.IsValid())
			{
				FEditorViewportClient& Viewport = ViewportWindow->GetAssetViewportClient();
				Viewport.EngineShowFlags.SetSingleFlag(ViewFlagIndex, IsJumpVisible);
			}
		}
	}
}

bool FJumpVisualizationModule::IsJumpVisualizationVisible() const
{
	return IsJumpVisible;
}

TArray<FPlayerJumpData> FJumpVisualizationModule::EditSessionJumpData(
	USimulationCharacterMovementComponent* SimChMovComp,
	const TArray<FPlayerJumpData>& SessionJumpData, const float& Angle, const FVector& Direction,
	const bool UseDifferentAirControl) const
{
	TArray<FPlayerJumpData> EditedJumpData;
	
	SimCh->SetActorLocation(SessionJumpData[0].Location);
	SimCh->SetActorRotation(SessionJumpData[0].Rotation);
	const FRotator Rotation(0.f, Angle, 0.f);
	const FVector RotatedDirection = Rotation.RotateVector(Direction);
	const FVector VisActorAcc = RotatedDirection * SessionJumpData[0].MaxAcceleration;
	for(int i = 0; i < 720 || i < SessionJumpData.Num(); i++)
	{
		if(SessionJumpData.Num() > i)
		{
			if(UseDifferentAirControl)
				SimChMovComp->SetAcceleration(VisActorAcc);
			else
				SimChMovComp->SetAcceleration(SessionJumpData[i].Acceleration);
		}
			
		SimChMovComp->PhysFalling(0.016f, 0);
		FPlayerJumpData PlayerJumpData;
		PlayerJumpData.TopMiddle = SimChMovComp->GetActorLocation();
		PlayerJumpData.BottomMiddle = SimChMovComp->GetActorLocation();
		PlayerJumpData.TopMiddle.Z += SessionJumpData[0].HalfCapsuleHeight;
		PlayerJumpData.BottomMiddle.Z -= SessionJumpData[0].HalfCapsuleHeight;
		PlayerJumpData.Location = SimChMovComp->GetActorLocation();
		EditedJumpData.Add(PlayerJumpData);
	}

	return EditedJumpData;
}

void FJumpVisualizationModule::RegisterMenuExtensions()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	UToolMenu* ShowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelViewportToolbar.Show");
	FToolMenuSection& ShowMenuSection = ShowMenu->FindOrAddSection("CustomLevelViewportEditorShow");
	ShowMenuSection.AddSeparator(FName("Custom Level Editor Tools"));
	const FUIAction Action (FExecuteAction::CreateRaw(this, &FJumpVisualizationModule::ToggleJumpVisualization),
	                        FCanExecuteAction(),
	                        FIsActionChecked::CreateRaw(this, &FJumpVisualizationModule::IsJumpVisualizationVisible));
	ShowMenuSection.AddMenuEntry(FName("JumpVisualizer"),
		FText::FromString("Jump Visualizer"),
		FText::FromString("Visualize Jumps From Previous Play"),
		FSlateIcon(),
		Action,
		EUserInterfaceActionType::ToggleButton);
	TSharedPtr<SComboButton> ComboButton = SNew(SComboButton).AccessibleText(LOCTEXT("Combo", ""));
}

void FJumpVisualizationModule::StartRecordingData(bool IsSimulating)
{
	AllJumpData.Empty();
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
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateRaw(this, &FJumpVisualizationModule::CollectResourceData);
	GEditor->GetTimerManager()->SetTimer(CollectResourceDataTimer, TimerDelegate, 0.016f, true, 0.f);

}

void FJumpVisualizationModule::CheckPlayerData()
{
	const APlayerController* PlayerControllerRef = UGameplayStatics::GetPlayerController(GEditor->PlayWorld, 0);
	if(!PlayerControllerRef)
		return;
	
	ACharacter* CharacterRef = Cast<ACharacter>(PlayerControllerRef->GetPawn());
	if(!CharacterRef)
		return;
	
	if(CharacterRef->bWasJumping)
	{
		FCoreDelegates::OnEndFrame.Remove(CheckJumpDelegate);
		AllJumpData.Add(TArray<FPlayerJumpData>());
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateRaw(this, &FJumpVisualizationModule::CollectJumpData, CharacterRef);
		GEditor->GetTimerManager()->SetTimer(CollectJumpDataTimer, TimerDelegate, 0.016f, true, 0.f);
	}
}

void FJumpVisualizationModule::CollectJumpData(ACharacter* Character)
{
	if(!Character)
		return;

	const UCharacterMovementComponent* MovementRef = Character->GetCharacterMovement();
	const UCapsuleComponent* CapsuleRef = Character->GetCapsuleComponent();
	if(!MovementRef || !CapsuleRef)
		return;
	
	FPlayerJumpData JumpData;
	float Radius = 0.f;
	float HalfHeight = 0.f;
	CapsuleRef->GetScaledCapsuleSize(Radius, HalfHeight);
	
	JumpData.Acceleration = MovementRef->GetCurrentAcceleration();
	JumpData.MaxAcceleration = MovementRef->GetMaxAcceleration();
	JumpData.MaxSimulationIterations = MovementRef->MaxSimulationIterations;
	JumpData.MaxSimulationTimeStep = MovementRef->MaxSimulationTimeStep;
	JumpData.AirControl = MovementRef->AirControl;
	JumpData.AirControlBoostMultiplier = MovementRef->AirControlBoostMultiplier;
	JumpData.AirControlBoostVelocityThreshold = MovementRef->AirControlBoostVelocityThreshold;
	JumpData.Location = CapsuleRef->GetComponentLocation();
	JumpData.HalfCapsuleHeight = HalfHeight;
	JumpData.TopMiddle = JumpData.BottomMiddle = CapsuleRef->GetComponentLocation();
	JumpData.TopMiddle.Z += HalfHeight;
	JumpData.BottomMiddle.Z -= HalfHeight;
	JumpData.Time = FDateTime::Now();
	JumpData.Velocity = Character->GetVelocity();
	JumpData.Speed = Character->GetVelocity().Size();
	JumpData.GravityScale = MovementRef->GravityScale;
	JumpData.JumpZVelocity = MovementRef->JumpZVelocity;
	
	AllJumpData.Last().Add(JumpData);
	if(!MovementRef->IsFalling())
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
	
	ACharacter* CharacterRef = Cast<ACharacter>(PlayerControllerRef->GetPawn());
	if(!CharacterRef)
		return;

	const UCharacterMovementComponent* MovementRef = CharacterRef->GetCharacterMovement();
	const UCapsuleComponent* CapsuleRef = CharacterRef->GetCapsuleComponent();
	if(!MovementRef || !CapsuleRef)
		return;
	
	float Radius = 0.f;
	float HalfHeight = 0.f;
	CapsuleRef->GetScaledCapsuleSize(Radius, HalfHeight);
	
	const FVector Location =  CapsuleRef->GetComponentLocation();
	const FDateTime Time = FDateTime::Now();
	for(auto& Resource : ResourceData)
	{
		Resource.Value.Emplace(*Resource.Key, Location, Time);
	}
}

void FJumpVisualizationModule::OnEndPIE(bool IsSimulating)
{
	FBufferArchive Archive;
	Archive << AllJumpData;
	
	const FDateTime CurrentTime = FDateTime::UtcNow();
	FString NewFileName = TEXT("JumpDataFile");
	NewFileName.Append(CurrentTime.ToString());
	NewFileName.Append(TEXT(".dat"));
	const FString FullPath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/"), NewFileName);

	TArray<FString> FileNames;
	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	while(FileNames.Num() > MaxJumpsRecorded - 1)
	{
		FString OldestFile;
		if(!GetOldestFile(OldestFile))
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

bool FJumpVisualizationModule::GetOldestFile(FString& FileName)
{
	FileName = "";
	TArray<FString> FileNames;
	TMap<FDateTime, FString> FileMap;
	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	if(!FileNames.Num())
		return false;

	for(int i = 0; i < FileNames.Num(); i++)
	{
		FString CurrentFileName = FileNames[i];
		CurrentFileName.RemoveFromEnd(".dat");
		FString TimeStampString = CurrentFileName.RightChop(18);

		FDateTime Timestamp;
		FDateTime::Parse(TimeStampString, Timestamp);

		FileMap.Add(Timestamp, FileNames[i]);
	}
	TArray<FDateTime> Keys;
	FileMap.GenerateKeyArray(Keys);
	
	FileName = FileMap[Keys[0]];
	return true;
}

bool FJumpVisualizationModule::GetNFile(const int N, FString& FileName)
{
	FileName = "";
	TArray<FString> FileNames;
	TMap<FDateTime, FString> FileMap;
	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	if(FileNames.Num() - N < 0)
		return false;
	
	for(int i = 0; i < FileNames.Num(); i++)
	{
		FString CurrentFileName = FileNames[i];
		CurrentFileName.RemoveFromEnd(".dat");
		FString TimeStampString = CurrentFileName.RightChop(18);
		
		FDateTime Timestamp;
		FDateTime::Parse(TimeStampString, Timestamp);

		FileMap.Add(Timestamp, FileNames[i]);
	}
	
	TArray<FDateTime> Keys;
	FileMap.GenerateKeyArray(Keys);
	const int FileIndex = Keys.Num() - N;
	for(int i = Keys.Num() - 1; i >= 0; i--)
	{
		if(i == FileIndex)
		{
        	FileName = FileMap[Keys[i]];
			return true;
		}
	}
	
	return false;
}

int FJumpVisualizationModule::GetAmountOfFiles()
{	
	TArray<FString> FileNames;
 	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	return FileNames.Num();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FJumpVisualizationModule, JumpVisualization)