// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DevNoteActor.generated.h"

class UWidgetComponent;
class UTextRenderComponent;

UCLASS()
class DEVNOTES_API ADevNoteActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADevNoteActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* NoteMesh;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	//UTextRenderComponent* NoteText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	UWidgetComponent* NoteWidget;

	void CheckClickedActor(const TArray<UObject*>& NewSelection, bool bForceRefresh);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void OpenTextEditWidget();
	void FitTextVertically(FString& Text);
	void FitTextHorizontally(FString& Text);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	FDateTime TimeLastClick = FDateTime::MinValue();
	bool IsEditOpen = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	FVector DefaultLocation = FVector::ZeroVector;
};
