#include "JumpVisComp.h"

#include "FJumpDebugSceneProxy.h"
#include "JumpVisActor.h"
#include "JumpVisualization.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"

namespace JumpVis {
	namespace Editor {

	// Editor show flag used to turn on and off encounter visualization components
	TCustomShowFlag<> ShowJumpVis(TEXT("JumpVisualization"), false /*DefaultEnabled*/, SFG_Developer, FText::FromString("Jump Visualization"));
	}
} // Ivy::Editor

UJumpVisComp::UJumpVisComp()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCastShadow(false);
	SetHiddenInGame(true);
	bVisibleInReflectionCaptures = false;
	bVisibleInRayTracing = false;
	bVisibleInRealTimeSkyCaptures = false;

	bIsEditorOnly = true;

#if WITH_EDITORONLY_DATA
	SetIsVisualizationComponent(true);
#endif;
}

FDebugRenderSceneProxy* UJumpVisComp::CreateDebugSceneProxy()
{
	FJumpDebugSceneProxy* DSceneProxy = new FJumpDebugSceneProxy(this);
	const AJumpVisActor* Owner = Cast<AJumpVisActor>(GetOwner());
	if(!Owner)
		return DSceneProxy;
	
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if(!World)
		World = GEditor->PlayWorld;
	if(World)
	{
		TArray<TArray<FCapsuleLocation>> JumpLocations;
		TArray<uint8> BinaryData;
		bool bFoundFile = false;
		FJumpVisualizationModule& JumpVisualizationModule = FModuleManager::GetModuleChecked<FJumpVisualizationModule>("JumpVisualization");
	
		FString FilePath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/"), FJumpVisualizationModule::GetNFile(bFoundFile, JumpVisualizationModule.GetSessionNumberToShow()));
		if(FFileHelper::LoadFileToArray(BinaryData, *FilePath) && bFoundFile)
		{
			FMemoryReader Archive(BinaryData, true);
			Archive.Seek(0);

			Archive << JumpLocations;

			if(!Owner->UseSetValues)
			{
				for(int j = 0; j < JumpLocations.Num(); j++)
				{
					if(JumpLocations[j].Num() < 2)
						continue;
					for(int i = 0; i < JumpLocations[j].Num() - 1; i++)
					{
						FVector Bottom1 = JumpLocations[j][i].BottomMiddle;
						FVector Top1 = JumpLocations[j][i].TopMiddle;
		
						FVector Bottom2 = JumpLocations[j][i + 1].BottomMiddle;
						FVector Top2 = JumpLocations[j][i + 1].TopMiddle;
				
						FLinearColor LineColor = FLinearColor::Red;
						FLinearColor LineColor2 = FLinearColor::Green;
						float LineThickness = 3.0f;
			
						DSceneProxy->Lines.Emplace(Bottom1, Bottom2, LineColor2.ToFColor(true), LineThickness);
						DSceneProxy->Lines.Emplace(Top1, Top2, LineColor2.ToFColor(true), LineThickness);
						DSceneProxy->Lines.Emplace(Bottom1, Top1, LineColor.ToFColor(true), LineThickness);
						DSceneProxy->Lines.Emplace(Bottom2, Top2, LineColor.ToFColor(true), LineThickness);
					}
				}
			}
			else
			{
				//TArray<TArray<FCapsuleLocation>> NewJumpLocations;
				//TArray<TArray<FCapsuleLocation>> NewLocations = JumpVisualizationModule.CalculateJumpLocation(JumpLocations);
				for(int j = 0; j < JumpLocations.Num(); j++)
				{
					if(JumpLocations[j].Num() < 2)
						continue;
					FPredictProjectilePathParams PredictParams;
					FPredictProjectilePathResult PredictResult;
					//Gravity Scale?
					//Speed - NewSpeed/Speed
					PredictParams.OverrideGravityZ = JumpLocations[j][0].GravityScale * World->GetGravityZ();
					PredictParams.LaunchVelocity = JumpLocations[j][0].Velocity;
					//PredictParams.LaunchVelocity.Z = 0.f;
					PredictParams.LaunchVelocity.Z = Owner->JumpZVelocity;// - JumpLocations[j][0].JumpZVelocity;
					PredictParams.StartLocation = JumpLocations[j][0].Location;
					PredictParams.MaxSimTime = 20.f;
					PredictParams.DrawDebugType = EDrawDebugTrace::None;

					UGameplayStatics::PredictProjectilePath(World, PredictParams, PredictResult);

					
					for(int i = 0; i < PredictResult.PathData.Num() - 1; i++)
					{
						if(PredictResult.PathData.Num() < 2)
							continue;

						FVector Bottom1 = PredictResult.PathData[i].Location;
						Bottom1.Z -= JumpLocations[j][0].HalfCapsuleHeight;
				
						FVector Top1 = PredictResult.PathData[i].Location;
						Top1.Z += JumpLocations[j][0].HalfCapsuleHeight;

						FVector Bottom2 = PredictResult.PathData[i + 1].Location;
						Bottom2.Z -= JumpLocations[j][0].HalfCapsuleHeight;
				
						FVector Top2 = PredictResult.PathData[i + 1].Location;
						Top2.Z += JumpLocations[j][0].HalfCapsuleHeight;

						FLinearColor LineColor = FLinearColor::Red;
						FLinearColor LineColor2 = FLinearColor::Green;
						float LineThickness = 3.0f;
			
						DSceneProxy->Lines.Emplace(Bottom1, Bottom2, LineColor2.ToFColor(true), LineThickness);
						DSceneProxy->Lines.Emplace(Top1, Top2, LineColor2.ToFColor(true), LineThickness);
						DSceneProxy->Lines.Emplace(Bottom1, Top1, LineColor.ToFColor(true), LineThickness);
						DSceneProxy->Lines.Emplace(Bottom2, Top2, LineColor.ToFColor(true), LineThickness);
					}
				}
			}
		}

		TMap<int*, TArray<FResourceData>> ResourceData = JumpVisualizationModule.GetResourceData();
		for(const auto Resource : ResourceData)
		{
			for(int i = 0; i < Resource.Value.Num(); i++)
			{
				//DSceneProxy->Texts.Emplace(FString::FromInt(Resource.Value[i].Amount), Resource.Value[i].Location, FLinearColor::Red);
			}
		}
	}

	return DSceneProxy;
}

FBoxSphereBounds UJumpVisComp::CalcBounds(const FTransform& Transform) const
{
	FBoxSphereBounds::Builder BoundsBuilder;
	BoundsBuilder += Super::CalcBounds(Transform);
	// Add initial sphere bounds so if we have no TestChildren our bounds will still be non-zero
	BoundsBuilder += FSphere(GetComponentLocation(), 50.f);
	return BoundsBuilder;
}



