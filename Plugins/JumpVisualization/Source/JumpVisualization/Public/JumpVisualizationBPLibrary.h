#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JumpVisualizationBPLibrary.generated.h"

/**
 * 
 */
UCLASS()
class JUMPVISUALIZATION_API UJumpVisualizationBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "JumpVisualization")
	static void SetJumpDataSessionNumber(const int SessionNumber);
	
};
