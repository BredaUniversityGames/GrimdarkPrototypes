// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CustomCharacter.h"

// Sets default values
ACustomCharacter::ACustomCharacter()
	: AGrimdarkPrototypesCharacter()
{
	
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACustomCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACustomCharacter::Jump()
{
	Super::Jump();
	
	OnJumpDelegate.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("JUMP BROADCAST"));
}

// Called every frame
void ACustomCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACustomCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

