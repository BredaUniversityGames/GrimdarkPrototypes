#pragma once

#include "PlayerJumpData.Generated.h"

USTRUCT()
struct JUMPVISUALIZATION_API FPlayerJumpData
{

	GENERATED_BODY()
	
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

inline FArchive& operator<<(FArchive& Ar, FPlayerJumpData& L)
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
