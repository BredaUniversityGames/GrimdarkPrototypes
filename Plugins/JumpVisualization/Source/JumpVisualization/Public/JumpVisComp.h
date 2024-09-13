// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

class JUMPVISUALIZATION_API UJumpVisComp : public FComponentVisualizer
{
public:	
	// Sets default values for this component's properties
	UJumpVisComp();
	void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
		
};
