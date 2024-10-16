// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <vector>

class USimulationCharacterMovementComponent;
class ASimJumpCharacter;
class AJumpVisActor;
struct FCapsuleLocation;

struct FResourceData
{
	int Amount;
	FVector Location;
	FDateTime Time;
	
	FResourceData()
	{
		Amount = 0;
		Location = FVector::ZeroVector;
		Time = FDateTime::MinValue();
	}

	FResourceData(int NAmount, FVector NLocation, FDateTime NTime)
	{
		Amount = NAmount;
		Location = NLocation;
		Time = NTime;
	}
};

class FJumpVisualizationModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TArray<TArray<FCapsuleLocation>> GetCapsuleLocations() const { return JumpLocations; }
	static FString GetOldestFile(bool& bFoundFile);
	static FString GetNewestFile(bool& bFoundFile);
	static FString GetNFile(bool& bFoundFile, const int N);
	static int GetAmountOfFiles();
	int GetSessionNumberToShow() const { return SessionNumberToShow; }
	void SetSessionNumberToShow(int NewSessionNumberToShow) { SessionNumberToShow = NewSessionNumberToShow; UE_LOG(LogTemp, Warning, TEXT("New Session: %i"), SessionNumberToShow)}
	TMap<int*, TArray<FResourceData>> GetResourceData() const { return ResourceData; }
	void AddResourceToTrack(int* Variable) { ResourceData.Add(Variable, TArray<FResourceData>()); }
	void CalculateJumpLocation(const TArray<TArray<FCapsuleLocation>>& SessionJumps, TArray<TArray<FCapsuleLocation>>& Output, const AJumpVisActor* VisActor);
	void FindAndModifyJumpLocations(const AJumpVisActor* VisActor);
	//void ModifyJumpLocations(USimulationCharacterMovementComponent SimChMovComp, TObjectPtr<ASimJumpCharacter> SimCh, );
	//static FString ReadStringFromFile(FString FilePath, bool& Success, FString& InfoMessage);
	//FString WriteStringToFile(FString FilePath, FString Text, bool& Success, FString& InfoMessage);
private:                             
	void ToggleJumpVisualization();
	bool IsJumpVisualizationVisible() const;
	void BindCommands();
	void RegisterMenuExtensions();
	void StartRecordingData(bool IsSimulating);
	void CheckPlayerData();
	void CollectJumpData(ACharacter* Character);
	void CollectResourceData();
	void PrintJumpLocations(bool IsSimulating);
	void OnEndPIE(bool IsSimulating);
	//void CheckFilesNumber();
public:
	TArray<TArray<FCapsuleLocation>> ModifiedJumpLocations;
	TArray<TPair<TArray<FCapsuleLocation>, TArray<FCapsuleLocation>>> AirControlJumpRange;
	bool ShowAirControlRange = false;
private:
	
	FTimerHandle CollectJumpDataTimer;
	FTimerHandle CollectResourceDataTimer;
	FDelegateHandle RecordJumpsDelegate;
	FDelegateHandle CheckJumpDelegate;
	FDelegateHandle DrawDebugJumpsDelegate;
	FDelegateHandle CheckResourceDelegate;
	bool IsJumpVisible = false;
	uint32 ViewFlagIndex = -1;
	FString ViewFlagName = "";
	TSharedPtr<FUICommandList> CommandList;
	AJumpVisActor* JumpVisActor = nullptr;
	uint8 SessionNumberToShow = 1;
	TArray<TArray<FCapsuleLocation>> JumpLocations;
	TMap<int*, TArray<FResourceData>> ResourceData;
	
	TObjectPtr<ASimJumpCharacter> SimCh = nullptr;
};
