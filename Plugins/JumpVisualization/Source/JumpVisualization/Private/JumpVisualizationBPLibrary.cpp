#include "JumpVisualizationBPLibrary.h"

#include "JumpVisualization.h"
#include "JumpVisActor.h"

void UJumpVisualizationBPLibrary::SetJumpDataSessionNumber(const int SessionNumber)
{
	FJumpVisualizationModule& JumpVisualizationModule = FModuleManager::GetModuleChecked<FJumpVisualizationModule>("JumpVisualization");
	if(JumpVisualizationModule.GetAmountOfFiles() < SessionNumber)
		return;
	JumpVisualizationModule.SetSessionNumberToShow(SessionNumber);
}

void UJumpVisualizationBPLibrary::AddVariableToTrack(int& Variable)
{
	FJumpVisualizationModule& JumpVisualizationModule = FModuleManager::GetModuleChecked<FJumpVisualizationModule>("JumpVisualization");
	JumpVisualizationModule.AddResourceToTrack(&Variable);
}

