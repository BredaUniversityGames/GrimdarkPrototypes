// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JumpVisActor.generated.h"

class FJumpVisualizationModule;
class UJumpVisComp;

USTRUCT()
struct FCapsuleLocation
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FVector TopMiddle{0.f};
	UPROPERTY()
	FVector BottomMiddle{0.f};
	UPROPERTY()
	FVector Location{0.f};
	UPROPERTY()
	float HalfCapsuleHeight = 0.f;
	UPROPERTY()
	float HalfCapsuleRadius = 0.f;
	
	UPROPERTY()
	FDateTime Time;
	UPROPERTY()
	float Speed = 0.f;
	UPROPERTY()
	FVector Velocity{0.f};
	UPROPERTY()
	float GravityScale = 1.f;
	UPROPERTY()
	float JumpZVelocity = 700.f;
	UPROPERTY()
	float DeltaTime = 0.f;
	UPROPERTY()
	FQuat Rotation = FQuat::Identity;
	UPROPERTY()
	FVector Acceleration{0.f};
	UPROPERTY()
	float MaxAcceleration{0.f};
	UPROPERTY()
	float MaxFlyDeceleration = 0.f;
	UPROPERTY()
	float AirControl = 0.f;
	UPROPERTY()
	float AirControlBoostMultiplier = 0.f;
	UPROPERTY()
	float AirControlBoostVelocityThreshold = 0.f;
	UPROPERTY()
	int32 MaxSimulationIterations = 2;
	UPROPERTY()
	float MaxSimulationTimeStep = 0.f;
	UPROPERTY()
	float JumpForceTimeRemaining = 0.f;
	UPROPERTY()
	bool bApplyGravityWhileJumping = false;
};

inline FArchive& operator<<(FArchive& Ar, FCapsuleLocation& L)
{
	Ar << L.TopMiddle << L.BottomMiddle << L.Location;
	Ar << L.HalfCapsuleHeight << L.HalfCapsuleRadius;
	Ar << L.JumpForceTimeRemaining << L.bApplyGravityWhileJumping;
	Ar << L.Time;
	Ar << L.Speed << L.Velocity << L.GravityScale << L.JumpZVelocity;
	Ar << L.DeltaTime;
	Ar << L.Rotation << L.Acceleration << L.MaxAcceleration << L.MaxFlyDeceleration;
	Ar << L.AirControl << L.AirControlBoostMultiplier << L.AirControlBoostVelocityThreshold;
	Ar << L.MaxSimulationIterations << L.MaxSimulationTimeStep;
	return Ar;
}


UCLASS()
class JUMPVISUALIZATION_API AJumpVisActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJumpVisActor();

protected:
	friend FJumpVisualizationModule;
	friend UJumpVisComp;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UJumpVisComp* JumpData;

public:
	UPROPERTY(EditAnywhere)
	bool UseSetValues = false;
	UPROPERTY(EditAnywhere)
	float GravityScale = 1.f;
	UPROPERTY(EditAnywhere)
	float Speed = 500.f;
	UPROPERTY(EditAnywhere)
	float JumpZVelocity = 700.f;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

};
