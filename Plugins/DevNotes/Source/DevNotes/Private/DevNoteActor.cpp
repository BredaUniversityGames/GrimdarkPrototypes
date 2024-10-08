// Fill out your copyright notice in the Description page of Project Settings.


#include "DevNoteActor.h"

#include "LevelEditor.h"
#include "Components/TextRenderComponent.h"
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

// Sets default values
ADevNoteActor::ADevNoteActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	NoteMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoteMesh"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	if(CubeMeshAsset.Succeeded())
	{
		NoteMesh->SetStaticMesh(CubeMeshAsset.Object);
	}

	NoteText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NoteText"));
	NoteText->SetupAttachment(NoteMesh);
	FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(FName("LevelEditor"));
	LevelEditor.OnActorSelectionChanged().AddUObject(this, &ADevNoteActor::CheckClickedActor);
	DefaultLocation = NoteText->GetRelativeLocation();
}

void ADevNoteActor::CheckClickedActor(const TArray<UObject*>& NewSelection, bool bForceRefresh)
{
	bool Selected = false;
	for(int i = 0; i < NewSelection.Num(); i++)
	{
		if(NewSelection[i] == this)
		{
			Selected = true;
			break;
		}
	}
	if(!Selected) return;
	const FDateTime CurrentTime = FDateTime::Now();
	UE_LOG(LogTemp, Warning, TEXT("Time: %f"), (CurrentTime - TimeLastClick).GetTotalSeconds());
	if((CurrentTime - TimeLastClick).GetTotalSeconds() < 1.1f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Double Click"));
		if(!IsEditOpen)
			OpenTextEditWidget();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Single Click"));
	}
	TimeLastClick = CurrentTime;
}

void ADevNoteActor::OpenTextEditWidget()
{
	//if (GEditor && GEditor->IsEditor())
	IsEditOpen = true;
	{
		const FString CurrentText = NoteText->Text.ToString();
		
		TSharedRef<SWindow> TextWindow = SNew(SWindow)
			.Title(FText::FromString("Edit Text"))
			.ClientSize(FVector2D(400, 100))
			.SupportsMinimize(false).SupportsMaximize(false);

		
		
		TSharedRef<SMultiLineEditableTextBox> EditableTextBox = SNew(SMultiLineEditableTextBox)
			.Text(FText::FromString(CurrentText))
			.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type CommitType)
			{
				if (CommitType == ETextCommit::OnCleared)
				{
					FString NewTextString = NewText.ToString();
					NoteText->SetText(NewText);
					int Lines = 1;
					for(int i = 0; i < NewTextString.Len(); i++)
					{
						if(NewTextString[i] == '\n')
						{
							Lines++;
						}
					}
					FVector NewLocation = DefaultLocation;
					NewLocation.Z -= (Lines - 1) * NoteText->WorldSize * NoteText->GetRelativeScale3D().Z;
					NoteText->SetRelativeLocation(NewLocation);
								//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Text Updated!"));
							}
						});

		
		TextWindow->SetContent(EditableTextBox);
		TextWindow->GetOnWindowClosedEvent().AddLambda([this](const TSharedRef<SWindow>&) { IsEditOpen = false; });
		FSlateApplication::Get().AddWindow(TextWindow);
	}
}

// Called when the game starts or when spawned
void ADevNoteActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADevNoteActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

