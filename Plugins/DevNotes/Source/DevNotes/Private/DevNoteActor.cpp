// Fill out your copyright notice in the Description page of Project Settings.


#include "DevNoteActor.h"

#include "LevelEditor.h"
#include "NoteTextWidget.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#include "Engine/Font.h"
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

	NoteWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NoteText"));
	NoteWidget->SetWidgetClass(TSubclassOf<UNoteTextWidget>());
	NoteWidget->SetupAttachment(NoteMesh);
	
	//NoteText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NoteText"));
	//NoteText->SetupAttachment(NoteMesh);
	FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(FName("LevelEditor"));
	LevelEditor.OnActorSelectionChanged().AddUObject(this, &ADevNoteActor::CheckClickedActor);
	//DefaultLocation = NoteText->GetRelativeLocation();
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
		UNoteTextWidget* NoteTextWidget = Cast<UNoteTextWidget>(NoteWidget->GetWidget());
		const FString CurrentText = NoteTextWidget->TextBox->GetText().ToString();//->Text.ToString();
		
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
					//NoteText->SetText(NewText);
					UNoteTextWidget* NoteTextWidget = Cast<UNoteTextWidget>(NoteWidget->GetWidget());
					if(NoteTextWidget)
					{
						NoteTextWidget->TextBox->SetText(NewText);
					}
					//bool TextFit = false;
					//while(!TextFit)
					//{
					//	FitTextHorizontally(NewTextString);
					//	NoteText->SetText(FText::FromString(NewTextString));
					//	FitTextVertically(NewTextString);
					//	TextFit = true;
					//}
					//int32 Height, Width;
					//NoteText->Font->GetStringHeightAndWidth(NewTextString, Height, Width);
					//float t = NoteText->WorldSize / Height;
					//UE_LOG(LogTemp, Warning, TEXT("W: %f and H: %f"), static_cast<float>(Width) * t, t);
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Text Updated!"));
				}
			});
		//FontDefaultLineSpacing
		//	NoteText->Font->spac
		
		TextWindow->SetContent(EditableTextBox);
		TextWindow->GetOnWindowClosedEvent().AddLambda([this](const TSharedRef<SWindow>&) { IsEditOpen = false; });
		FSlateApplication::Get().AddWindow(TextWindow);
	}
}

void ADevNoteActor::FitTextVertically(FString& Text)
{
	//int Lines = 1;                                                                        
	//for(int i = 0; i < Text.Len(); i++)                                          
	//{                                                                                     
	//	if(Text[i] == '\n')                                                      
	//	{                                                                                 
	//		Lines++;                                                                      
	//	}                                                                                 
	//}                                                                                     
	//float NoteHeight = NoteMesh->Bounds.BoxExtent.Z * 2.f;                                
	//float NewWorldSize = NoteHeight / (Lines * NoteText->GetRelativeScale3D().Z);         
	//NoteText->WorldSize = NewWorldSize;                                                   
    //                                                                                  
	//FVector NewLocation = DefaultLocation;                                                
	//float MaxHeight = NoteText->Font->GetMaxCharHeight();                                 
	//NewLocation.Z = NoteHeight / 2.f - NewWorldSize;                                      
	//NewLocation.Z -= (Lines - 1) * NoteText->WorldSize * NoteText->GetRelativeScale3D().Z;
	//NoteText->SetRelativeLocation(NewLocation);                                           
}

void ADevNoteActor::FitTextHorizontally(FString& Text)
{
	int Lines = 0;
	FString FinalText = "";
	FString NewText = Text;
	int NewLineIndex = 0;
	float NoteWidth = NoteMesh->Bounds.BoxExtent.X * 2.f;
	while(NewText.FindChar('\n', NewLineIndex))
	{
		//int TextHeight, TextWidth;
		//FString CurrentLine = NewText.Left(NewLineIndex);
		//NoteText->Font->GetStringHeightAndWidth("a", TextHeight, TextWidth);
		//const float Scale = NoteText->WorldSize / TextHeight;
		//TextWidth *= Scale;
		//if(TextWidth > NoteWidth)
		//{
		//	
		//}
		//int TextHeight, TextWidth;
		//FString CurrentLine = NewText.Left(NewLineIndex);
		//NoteText->Font->GetStringHeightAndWidth(CurrentLine.Mid(0, NewLineIndex), TextHeight, TextWidth);
		//const float Scale = NoteText->WorldSize / TextHeight;
		//TextWidth *= Scale;
		//if(TextWidth > NoteWidth)
		//{
		//	bool CorrectSeparation = false;
		//	FString CurrentLineCopy = CurrentLine;
		//	TArray<int> SpaceIndices;
		//	int SpaceIndex = 0;
		//	int WordCount = 1;
		//	FString NewLine = "";
		//	while(CurrentLineCopy.FindChar(' ', SpaceIndex))
		//	{
		//		WordCount++;
		//		SpaceIndices.Add(SpaceIndex);
		//		int NewLineTextHeight, NewLineTextWidth;
		//		FString CurrentWord = CurrentLineCopy.Mid(0, SpaceIndex);
		//		NoteText->Font->GetStringHeightAndWidth(NewLine + CurrentWord, NewLineTextHeight, NewLineTextWidth);
		//		NewLineTextWidth *= Scale;
		//		if(NewLineTextWidth > NoteWidth)
		//		{
		//			FinalText += '\n';
		//			FinalText += CurrentWord;
		//			FinalText += ' ';
		//			NewLine = "";
		//		}
		//		else
		//		{
		//			FinalText += CurrentWord;
		//			FinalText += ' ';
		//			NewLine += CurrentWord;
		//			NewLine += " ";
		//		}
		//		CurrentLineCopy = CurrentLineCopy.RightChop(SpaceIndex + 1);
		//		//CurrentLineCopy = CurrentLineCopy.RightChop(SpaceIndex + 1);
		//	}
		//	FinalText += '\n';
		//	SpaceIndices.Add(CurrentLine.Len() - 1);
		//}
		//NewText = NewText.RightChop(NewLineIndex + 1);
	}
	Text = FinalText;
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

