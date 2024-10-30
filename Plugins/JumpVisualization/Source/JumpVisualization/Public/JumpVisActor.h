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

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
	
protected:
	friend FJumpVisualizationModule;
	friend UJumpVisComp;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere)
	bool UseSetValues = false;
	UPROPERTY(EditAnywhere)
	float GravityScale = 1.f;
	UPROPERTY(EditAnywhere)
	float Speed = 500.f;
	UPROPERTY(EditAnywhere)
	float JumpZVelocity = 700.f;
	UPROPERTY(EditAnywhere)
	bool UseDifferentAirControl = false;
	UPROPERTY(EditAnywhere, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float AirControlDirection = 0.f;
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AirControl = 0.f;
	UPROPERTY(EditAnywhere)
	bool ShowAirControlRange = false;

protected:
	UJumpVisComp* JumpData;
};
