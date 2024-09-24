// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpSimActor.h"

#include "JumpSimComp.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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

#if WITH_EDITOR
void AJumpSimActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if(PropertyName == GET_MEMBER_NAME_CHECKED(AJumpSimActor, CharacterClass))
	{
		TakeValuesFromClass();
	}
}
#endif

void AJumpSimActor::TakeValuesFromClass()
{
	if(!CharacterClass || !*CharacterClass)
		return;
	const UCharacterMovementComponent* CharacterMovComp = CharacterClass->GetDefaultObject<ACharacter>()->GetCharacterMovement();
	GravityScale = CharacterMovComp->GravityScale;
	JumpZVelocity = CharacterMovComp->JumpZVelocity;
	Speed = CharacterMovComp->MaxWalkSpeed;
}
