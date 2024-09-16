// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "Debug/DebugDrawComponent.h"

#include "JumpVisComp.generated.h"

UCLASS()
class JUMPVISUALIZATION_API UJumpVisComp : public UDebugDrawComponent
{
	GENERATED_BODY()
public:
	TArray<FVector> Locations;
	// Sets default values for this component's properties
	UJumpVisComp();
	virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& Transform) const override;
	virtual bool ShouldRecreateProxyOnUpdateTransform() const override {return true;};
};
