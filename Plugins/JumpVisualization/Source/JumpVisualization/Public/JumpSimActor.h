// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JumpSimActor.generated.h"

class UJumpSimComp;

UCLASS()
class JUMPVISUALIZATION_API AJumpSimActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJumpSimActor();
	
public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere)
	float GravityScale = 1.f;
	UPROPERTY(EditAnywhere)
	float MaxSpeed = 500.f;
	UPROPERTY(EditAnywhere)
	float Mass = 100.f;
	UPROPERTY(EditAnywhere)
	float JumpZVelocity = 700.f;
	UPROPERTY(EditAnywhere)
	float BrakingDeceleration = 0.f;
	UPROPERTY(EditAnywhere)
	float SimTime = 10.f;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UJumpSimComp* JumpSimComp;
	
};