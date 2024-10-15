// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BugReportLibrary.generated.h"

/**
 * 
 */
UCLASS()
class DEVNOTES_API UBugReportLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BugReport")
	static bool CreateJiraBug(const FString& BugName, const FString& BugDescription, const FString& Priority);
};
