// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DevNoteActor.generated.h"

class UTextRenderComponent;

UCLASS()
class DEVNOTES_API ADevNoteActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADevNoteActor();

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* NoteMesh;
	UPROPERTY(EditAnywhere, Category = "Text")
	UTextRenderComponent* NoteText;

	void CheckClickedActor(const TArray<UObject*>& NewSelection, bool bForceRefresh);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void OpenTextEditWidget();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	FDateTime TimeLastClick = FDateTime::MinValue();
	bool IsEditOpen = false;
};
