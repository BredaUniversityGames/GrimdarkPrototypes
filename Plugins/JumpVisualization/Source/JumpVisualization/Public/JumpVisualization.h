// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <vector>

struct CapsuleLocation
{
	FVector TopMiddle{0.f};
	FVector BottomMiddle{0.f};
};

class FJumpVisualizationModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	std::vector<CapsuleLocation> JumpLocations;
	
private:
	void ToggleJumpVisualization();
	bool IsJumpVisualizationVisible() const;
	void BindCommands();
	void RegisterMenuExtensions();
	void StartRecordingJumps(bool IsSimulating);
	void DidCharacterJustJump();
	void CollectJumpData(ACharacter* Character);
	void PrintJumpLocations(bool IsSimulating);
	void OnDrawJumpDebug(UCanvas* Canvas, APlayerController* PC);
public:
	
private:
	FTimerHandle CollectJumpDataTimer;
	FDelegateHandle RecordJumpsDelegate;
	FDelegateHandle CheckJumpDelegate;
	FDelegateHandle DrawDebugJumpsDelegate;
	bool IsJumpVisible = false;
	TSharedPtr<FUICommandList> CommandList;
};
