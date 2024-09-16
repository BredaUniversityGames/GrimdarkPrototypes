// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JumpVisActor.generated.h"

class FJumpVisualizationModule;
class UJumpVisComp;

UCLASS()
class JUMPVISUALIZATION_API AJumpVisActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJumpVisActor();

protected:
	friend FJumpVisualizationModule;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UJumpVisComp* JumpData;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; };

};
