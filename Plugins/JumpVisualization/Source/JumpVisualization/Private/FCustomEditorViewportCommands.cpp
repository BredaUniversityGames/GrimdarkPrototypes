#include "FCustomEditorViewportCommands.h"

#define LOCTEXT_NAMESPACE "FCustomEditorViewportCommands"

FCustomEditorViewportCommands::FCustomEditorViewportCommands()
	: TCommands<FCustomEditorViewportCommands>
	(
		TEXT("CustomEditorViewport"),
		NSLOCTEXT("Contexts", "CustomEditorViewportCommands", "Custom Editor Viewport Commands"),
		TEXT("MainFrame"),
		FAppStyle::GetAppStyleSetName()
	)
{
		
}

void FCustomEditorViewportCommands::RegisterCommands()
{
	UI_COMMAND(JumpVisualization, "Jump Visualization", "Visualize the jumps from the previous playtime", EUserInterfaceActionType::ToggleButton, FInputChord());
}

#undef LOCTEXT_NAMESPACE