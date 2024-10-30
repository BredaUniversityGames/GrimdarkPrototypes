#pragma once

#include "TrackedResourceData.Generated.h"

USTRUCT()
struct JUMPVISUALIZATION_API FTrackedResourceData
{
	GENERATED_BODY()

	UPROPERTY()
	int Amount = 0;
	UPROPERTY()
	FVector Location = FVector::ZeroVector;
	UPROPERTY()
	FDateTime Time = FDateTime::MinValue();

	FTrackedResourceData() {}

	FTrackedResourceData(const int NAmount, const FVector& NLocation, const FDateTime NTime)
		: Amount(NAmount), Location(NLocation), Time(NTime)
	{}
};
