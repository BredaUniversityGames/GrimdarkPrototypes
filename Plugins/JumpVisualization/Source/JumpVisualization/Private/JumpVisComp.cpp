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
		FJumpVisualizationModule& JumpVisualizationModule = FModuleManager::GetModuleChecked<FJumpVisualizationModule>("JumpVisualization");
		TArray<TArray<FCapsuleLocation>> JumpLocations = JumpVisualizationModule.ModifiedJumpLocations;
		
		{
			{
				for(int j = 0; j < JumpLocations.Num(); j++)
				{
					if(JumpLocations[j].Num() < 2)
						continue;
					FVector PrevBottom = JumpLocations[j][0].BottomMiddle;
					FVector PrevTop = JumpLocations[j][0].TopMiddle;
					for(int i = 3; i < JumpLocations[j].Num() - 1; i += 3)
					{
						FVector NextBottom = JumpLocations[j][i + 1].BottomMiddle;
						FVector NextTop = JumpLocations[j][i + 1].TopMiddle;
				
						FLinearColor LineColor = FLinearColor::Red;
						FLinearColor LineColor2 = FLinearColor::Green;
						float LineThickness = 3.0f;
			
						DSceneProxy->Lines.Emplace(PrevBottom, NextBottom, LineColor2.ToFColor(true), LineThickness);
						DSceneProxy->Lines.Emplace(PrevTop, NextTop, LineColor2.ToFColor(true), LineThickness);
						DSceneProxy->Lines.Emplace(PrevBottom, PrevTop, LineColor.ToFColor(true), LineThickness);
						DSceneProxy->Lines.Emplace(NextBottom, NextTop, LineColor.ToFColor(true), LineThickness);
						
						PrevBottom = NextBottom;
						PrevTop = NextTop;
					}
				}

				if(JumpVisualizationModule.ShowAirControlRange)
				{
					TArray<TPair<TArray<FCapsuleLocation>, TArray<FCapsuleLocation>>> AirControlJumpRange = JumpVisualizationModule.AirControlJumpRange;
					for(int j = 0; j < AirControlJumpRange.Num(); j++)
					{
						if(AirControlJumpRange[j].Key.Num() < 2)
							continue;
						FVector PrevBottom = AirControlJumpRange[j].Key[0].BottomMiddle;
						FVector PrevTop = AirControlJumpRange[j].Key[0].TopMiddle;
						for(int i = 3; i < AirControlJumpRange[j].Key.Num() - 1; i += 3)
						{
							FVector NextBottom = AirControlJumpRange[j].Key[i + 1].BottomMiddle;
							FVector NextTop = AirControlJumpRange[j].Key[i + 1].TopMiddle;
				
							FLinearColor LineColor = FLinearColor::Black;
							FLinearColor LineColor2 = FLinearColor::Yellow;
							float LineThickness = 3.0f;
			
							DSceneProxy->Lines.Emplace(PrevBottom, NextBottom, LineColor2.ToFColor(true), LineThickness);
							DSceneProxy->Lines.Emplace(PrevTop, NextTop, LineColor2.ToFColor(true), LineThickness);
							DSceneProxy->Lines.Emplace(PrevBottom, PrevTop, LineColor.ToFColor(true), LineThickness);
							DSceneProxy->Lines.Emplace(NextBottom, NextTop, LineColor.ToFColor(true), LineThickness);
						
							PrevBottom = NextBottom;
							PrevTop = NextTop;
						}

						PrevBottom = AirControlJumpRange[j].Value[0].BottomMiddle;
						PrevTop = AirControlJumpRange[j].Value[0].TopMiddle;
						for(int i = 3; i < AirControlJumpRange[j].Key.Num() - 1; i += 3)
						{
							FVector NextBottom = AirControlJumpRange[j].Value[i + 1].BottomMiddle;
							FVector NextTop = AirControlJumpRange[j].Value[i + 1].TopMiddle;
				
							FLinearColor LineColor = FLinearColor::Black;
							FLinearColor LineColor2 = FLinearColor::Yellow;
							float LineThickness = 3.0f;
			
							DSceneProxy->Lines.Emplace(PrevBottom, NextBottom, LineColor2.ToFColor(true), LineThickness);
							DSceneProxy->Lines.Emplace(PrevTop, NextTop, LineColor2.ToFColor(true), LineThickness);
							DSceneProxy->Lines.Emplace(PrevBottom, PrevTop, LineColor.ToFColor(true), LineThickness);
							DSceneProxy->Lines.Emplace(NextBottom, NextTop, LineColor.ToFColor(true), LineThickness);
						
							PrevBottom = NextBottom;
							PrevTop = NextTop;
						}
					}
				}
			}
			//else
			//{
			//	//TArray<TArray<FCapsuleLocation>> NewJumpLocations;
			//	TArray<TArray<FCapsuleLocation>> NewJumpLocations;
			//	JumpVisualizationModule.CalculateJumpLocation(JumpLocations, NewJumpLocations);
			//	//FTimerDelegate TimerDelegate = FTimerDelegate::CreateRaw(&JumpVisualizationModule, &FJumpVisualizationModule::CalculateJumpLocation, JumpLocations, NewJumpLocations);
			//	//GetWorld()->GetTimerManager().SetTimer(
			//	//	ResetTimer,
			//	//	[&JumpVisualizationModule, &JumpLocations, &NewJumpLocations]()
			//	//	{
			//	//		JumpVisualizationModule.CalculateJumpLocation(JumpLocations, NewJumpLocations);
			//	//	},
			//	//	0.05f,   // Delay in seconds
			//	//	false    // Single execution (not looping)
			//	//);
			//	//GEditor->GetTimerManager()->SetTimerForNextTick(TimerDelegate);
			//	for(int j = 0; j < NewJumpLocations.Num(); j++)
			//	{
			//		if(NewJumpLocations[j].Num() < 2)
			//			continue;
			//		//FPredictProjectilePathParams PredictParams;
			//		//FPredictProjectilePathResult PredictResult;
			//		////Gravity Scale?
			//		////Speed - NewSpeed/Speed
			//		//PredictParams.OverrideGravityZ = NewJumpLocations[j][0].GravityScale * World->GetGravityZ();
			//		//PredictParams.LaunchVelocity = NewJumpLocations[j][0].Velocity;
			//		////PredictParams.LaunchVelocity.Z = 0.f;
			//		//PredictParams.LaunchVelocity.Z = Owner->JumpZVelocity;// - JumpLocations[j][0].JumpZVelocity;
			//		//PredictParams.StartLocation = NewJumpLocations[j][0].Location;
			//		//PredictParams.MaxSimTime = 20.f;
			//		//PredictParams.DrawDebugType = EDrawDebugTrace::None;
////
			//		//UGameplayStatics::PredictProjectilePath(World, PredictParams, PredictResult);
//
			//		
			//		for(int i = 0; i < NewJumpLocations[j].Num() - 1; i++)
			//		{
			//			FVector Bottom1 = NewJumpLocations[j][i].BottomMiddle;
			//			//Bottom1.Z -= JumpLocations[j][0].HalfCapsuleHeight;
			//	
			//			FVector Top1 = NewJumpLocations[j][i].TopMiddle;
			//			//Top1.Z += JumpLocations[j][0].HalfCapsuleHeight;
//
			//			FVector Bottom2 = NewJumpLocations[j][i + 1].BottomMiddle;
			//			//Bottom2.Z -= JumpLocations[j][0].HalfCapsuleHeight;
			//	
			//			FVector Top2 = NewJumpLocations[j][i + 1].TopMiddle;
			//			//Top2.Z += JumpLocations[j][0].HalfCapsuleHeight;
//
			//			FLinearColor LineColor = FLinearColor::Red;
			//			FLinearColor LineColor2 = FLinearColor::Green;
			//			float LineThickness = 3.0f;
			//
			//			DSceneProxy->Lines.Emplace(Bottom1, Bottom2, LineColor2.ToFColor(true), LineThickness);
			//			DSceneProxy->Lines.Emplace(Top1, Top2, LineColor2.ToFColor(true), LineThickness);
			//			DSceneProxy->Lines.Emplace(Bottom1, Top1, LineColor.ToFColor(true), LineThickness);
			//			DSceneProxy->Lines.Emplace(Bottom2, Top2, LineColor.ToFColor(true), LineThickness);
			//		}
			//	}
			//}
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



