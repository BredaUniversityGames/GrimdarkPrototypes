// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NoteTextWidget.generated.h"

class UScaleBox;
class USizeBox;
class UMultiLineEditableTextBox;
/**
 * 
 */
UCLASS()
class DEVNOTES_API UNoteTextWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NoteTextWidget", meta = (BindWidget))
	//UScaleBox* ScaleBox;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NoteTextWidget", meta = (BindWidget))
	USizeBox* SizeBox;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NoteTextWidget", meta = (BindWidget))
	UMultiLineEditableTextBox* TextBox;
};
