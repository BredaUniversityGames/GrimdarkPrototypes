// Fill out your copyright notice in the Description page of Project Settings.


#include "BugReportButtonStyle.h"

#include "DevNotes.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FBugReportButtonStyle::StyleInstance = nullptr;

void FBugReportButtonStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FBugReportButtonStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FBugReportButtonStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("BugReportButtonStyle"));
	return StyleSetName;
}

void FBugReportButtonStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedRef<FSlateStyleSet> FBugReportButtonStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("BugReportButtonStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("DevNotes")->GetBaseDir() / TEXT("Resources"));
	//const FVector2D Icon16x16{16.f, 16.f};
	const FVector2D Icon20x20{20.f, 20.f};
	Style->Set("BugReport.BugReportAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

const ISlateStyle& FBugReportButtonStyle::Get()
{
	return *StyleInstance;
}