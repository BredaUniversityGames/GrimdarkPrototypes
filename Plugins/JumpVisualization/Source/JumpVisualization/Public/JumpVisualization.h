// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "PlayerJumpData.h"
#include "TrackedResourceData.h"

class USimulationCharacterMovementComponent;
class ASimJumpCharacter;
class AJumpVisActor;

//struct FPlayerJumpData;
//struct FTrackedResourceData;

class JUMPVISUALIZATION_API FJumpVisualizationModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static bool GetOldestFile(FString& FileName);
	static bool GetNFile(int N, FString& FileName);
	static int GetAmountOfFiles();
	int GetSessionNumberToShow() const { return SessionNumberToShow; }
	bool GetShowAirControlRange() const { return ShowAirControlRange; }
	TArray<TArray<FPlayerJumpData>> GetCapsuleLocations() const { return AllJumpData; }
	TMap<int*, TArray<FTrackedResourceData>> GetResourceData() const { return ResourceData; }

	void AddResourceToTrack(int* Variable) { ResourceData.Add(Variable, TArray<FTrackedResourceData>()); }
	void FindAndModifyJumpLocations(const AJumpVisActor* VisActor);
	void SetSessionNumberToShow(const int NewSessionNumberToShow) { SessionNumberToShow = NewSessionNumberToShow; UE_LOG(LogTemp, Warning, TEXT("New Session: %i"), SessionNumberToShow)}

private:                             
	void ToggleJumpVisualization();
	bool IsJumpVisualizationVisible() const;
	TArray<FPlayerJumpData> EditSessionJumpData(USimulationCharacterMovementComponent* SimChMovComp,
	                                            const TArray<FPlayerJumpData>& SessionJumpData, const float& Angle,
	                                            const FVector& Direction, bool
	                                            UseDifferentAirControl) const;
	void CalculateJumpLocation(const TArray<TArray<FPlayerJumpData>>& SessionJumps, TArray<TArray<FPlayerJumpData>>& Output, const AJumpVisActor* VisActor);
	void RegisterMenuExtensions();
	void StartRecordingData(bool IsSimulating);
	void CheckPlayerData();
	void CollectJumpData(ACharacter* Character);
	void CollectResourceData();
	void OnEndPIE(bool IsSimulating);
	
public:
	TArray<TArray<FPlayerJumpData>> ModifiedJumpLocations;
	TArray<TPair<TArray<FPlayerJumpData>, TArray<FPlayerJumpData>>> AirControlJumpRange;
	
private:
	FTimerHandle CollectJumpDataTimer;
	FTimerHandle CollectResourceDataTimer;
	FDelegateHandle RecordJumpsDelegate;
	FDelegateHandle CheckJumpDelegate;
	FDelegateHandle DrawDebugJumpsDelegate;
	FDelegateHandle CheckResourceDelegate;
	
	AJumpVisActor* JumpVisActor = nullptr;
	TObjectPtr<ASimJumpCharacter> SimCh = nullptr;
	bool IsJumpVisible = false;
	bool ShowAirControlRange = false;
	bool IsRecordingJumpData = false;
	int SessionNumberToShow = 1;
	int MaxJumpsRecorded = 100; 
	uint32 ViewFlagIndex = -1;
	FString ViewFlagName = "";
	TArray<TArray<FPlayerJumpData>> AllJumpData;
	TMap<int*, TArray<FTrackedResourceData>> ResourceData;
};
