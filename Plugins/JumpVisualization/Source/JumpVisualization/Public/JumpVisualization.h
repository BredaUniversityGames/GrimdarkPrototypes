// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <vector>

class AJumpVisActor;
struct FCapsuleLocation;

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
	//static FString ReadStringFromFile(FString FilePath, bool& Success, FString& InfoMessage);
	//FString WriteStringToFile(FString FilePath, FString Text, bool& Success, FString& InfoMessage);
private:                             
	void ToggleJumpVisualization();
	bool IsJumpVisualizationVisible() const;
	void BindCommands();
	void RegisterMenuExtensions();
	void StartRecordingJumps(bool IsSimulating);
	void DidCharacterJustJump();
	void CollectJumpData(ACharacter* Character);
	void PrintJumpLocations(bool IsSimulating);
	void OnEndPIE(bool IsSimulating);
	//void CheckFilesNumber();
public:
	
private:
	
	FTimerHandle CollectJumpDataTimer;
	FDelegateHandle RecordJumpsDelegate;
	FDelegateHandle CheckJumpDelegate;
	FDelegateHandle DrawDebugJumpsDelegate;
	bool IsJumpVisible = false;
	uint32 ViewFlagIndex = -1;
	FString ViewFlagName = "";
	TSharedPtr<FUICommandList> CommandList;
	AJumpVisActor* JumpVisActor = nullptr;
	TMap<int, TArray<FCapsuleLocation>> Test;
	uint8 SessionNumberToShow = 1;
	TArray<TArray<FCapsuleLocation>> JumpLocations;
};
