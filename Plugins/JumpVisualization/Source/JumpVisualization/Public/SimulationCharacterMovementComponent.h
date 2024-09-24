// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SimulationCharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JUMPVISUALIZATION_API USimulationCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USimulationCharacterMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void SetAcceleration(const FVector& NewAcceleration) { Acceleration = NewAcceleration; }
	void SetCharacterOwner(const TObjectPtr<ACharacter> Character) { CharacterOwner = Character; }
	void SetUpdatedComponent(const TObjectPtr<USceneComponent> SceneComponent) { UpdatedComponent = SceneComponent; }
};
