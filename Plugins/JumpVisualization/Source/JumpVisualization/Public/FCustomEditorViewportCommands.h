#pragma once

#include "Framework/Commands/Commands.h"

class FUICommandList;

class FCustomEditorViewportCommands : public TCommands<FCustomEditorViewportCommands>
{
public:
	FCustomEditorViewportCommands();

	void RegisterCommands() override;
	TSharedPtr<FUICommandInfo> JumpVisualization;
};
