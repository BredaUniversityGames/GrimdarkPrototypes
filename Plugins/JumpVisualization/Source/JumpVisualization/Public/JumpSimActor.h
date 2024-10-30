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
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void TakeValuesFromClass();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	bool GetTrackCharacterValues() const { return TrackCharacterValues; }
	float GetGravityScale() const { return GravityScale; }
	float GetSpeed() const { return Speed; }
	float GetJumpZVelocity() const { return JumpZVelocity; }
	float GetSimTime() const { return SimTime; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACharacter> CharacterClass;
	UPROPERTY(EditAnywhere)
	bool TrackCharacterValues = false;
	UPROPERTY(EditAnywhere)
	float GravityScale = 1.f;
	UPROPERTY(EditAnywhere)
	float Speed = 500.f;
	UPROPERTY(EditAnywhere)
	float JumpZVelocity = 700.f;
	
	float SimTime = 20.f;
	UJumpSimComp* JumpSimComp;
};
