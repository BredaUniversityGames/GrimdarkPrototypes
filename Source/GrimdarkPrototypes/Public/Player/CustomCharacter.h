// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrimdarkPrototypes/GrimdarkPrototypesCharacter.h"
#include "CustomCharacter.generated.h"

UCLASS()
class GRIMDARKPROTOTYPES_API ACustomCharacter : public AGrimdarkPrototypesCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACustomCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJumpDelegate);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnJumpDelegate OnJumpDelegate;
	
	virtual void Jump() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
