// Fill out your copyright notice in the Description page of Project Settings.


#include "SimJumpCharacter.h"

#include "SimulationCharacterMovementComponent.h"


// Sets default values
ASimJumpCharacter::ASimJumpCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USimulationCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void ASimJumpCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASimJumpCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASimJumpCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

