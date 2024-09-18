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
	FDateTime Time;

};

inline FArchive& operator<<(FArchive& Ar, FCapsuleLocation& L)
{
	Ar << L.TopMiddle << L.BottomMiddle;
	Ar << L.Time;
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
	TArray<TArray<FCapsuleLocation>> JumpLocations;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

};
