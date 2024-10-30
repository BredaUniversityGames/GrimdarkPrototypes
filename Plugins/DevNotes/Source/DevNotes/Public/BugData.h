#pragma once

#include "BugData.generated.h"

USTRUCT(BlueprintType)
struct DEVNOTES_API FBugData
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	FString Key = "";
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	FString Summary = "";
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	FString Description = "";
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	FString Priority = "";
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	FString Assignee = "";
	
	FBugData(){};
	FBugData(const FString& InKey, const FString& InSummary, const FString& InDescription, const FString& InPriority, const FString& InAssignee)
		: Key(InKey), Summary(InSummary), Description(InDescription), Priority(InPriority), Assignee(InAssignee)
	{};
};
