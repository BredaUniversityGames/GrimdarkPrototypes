// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FSlateStyleSet;

/**
 * 
 */
class DEVNOTES_API FBugReportButtonStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static void ReloadTextures();
	static const ISlateStyle& Get();
	static FName GetStyleSetName();

private:
	static TSharedRef<FSlateStyleSet> Create();

private:
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};