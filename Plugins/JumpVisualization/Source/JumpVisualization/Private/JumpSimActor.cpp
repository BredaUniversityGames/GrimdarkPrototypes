// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpSimActor.h"

#include "JumpSimComp.h"

// Sets default values
AJumpSimActor::AJumpSimActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	JumpSimComp = CreateDefaultSubobject<UJumpSimComp>(TEXT("JumpSimComponent"));
}

// Called when the game starts or when spawned
void AJumpSimActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AJumpSimActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

