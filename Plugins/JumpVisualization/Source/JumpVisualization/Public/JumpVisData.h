// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <vector>
#include "JumpVisData.generated.h"

class FJumpVisualizationModule;
class UJumpVisComp;
	
struct CapsuleLocation
{
	FVector TopMiddle{0.f};
	FVector BottomMiddle{0.f};
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JUMPVISUALIZATION_API UJumpVisData : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UJumpVisData();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	friend FJumpVisualizationModule;
	friend UJumpVisComp;

	std::vector<CapsuleLocation> JumpLocations;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
		
};
