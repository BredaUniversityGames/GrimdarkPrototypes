// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpVisActor.h"

#include "JumpVisComp.h"
#include "JumpVisData.h"
#include "LevelEditorViewport.h"

// Sets default values
AJumpVisActor::AJumpVisActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f;
	JumpData = CreateDefaultSubobject<UJumpVisComp>(TEXT("JumpVisComponent"));
}

// Called when the game starts or when spawned
void AJumpVisActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AJumpVisActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	GEditor->MoveActorInFrontOfCamera(*this, GCurrentLevelEditingViewportClient->GetViewLocation(), 
				  GCurrentLevelEditingViewportClient->GetViewRotation().Vector());

}

