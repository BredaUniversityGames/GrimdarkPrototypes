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
	TSharedPtr<FUICommandList> CommandList;
	AJumpVisActor* JumpVisActor = nullptr;
	TMap<int, TArray<FCapsuleLocation>> Test;
	uint8 CurrentIndex = 0;
	TArray<TArray<FCapsuleLocation>> JumpLocations;
};
