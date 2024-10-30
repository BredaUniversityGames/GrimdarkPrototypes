#include "JumpSimComp.h"

#include "FJumpDebugSceneProxy.h"
#include "JumpSimActor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UJumpSimComp::UJumpSimComp()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCastShadow(false);
	SetHiddenInGame(true);
	bVisibleInReflectionCaptures = false;
	bVisibleInRayTracing = false;
	bVisibleInRealTimeSkyCaptures = false;
	MarkRenderInstancesDirty();

#if WITH_EDITORONLY_DATA
	SetIsVisualizationComponent(true);
#endif
}

FDebugRenderSceneProxy* UJumpSimComp::CreateDebugSceneProxy()
{
	FJumpDebugSceneProxy* DSceneProxy = new FJumpDebugSceneProxy(this);
	AJumpSimActor* Owner = Cast<AJumpSimActor>(GetOwner());
	if(!Owner)
		return DSceneProxy;

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if(!World)
		World = GEditor->PlayWorld;
	if(World)
	{
		FPredictProjectilePathParams PredictParams;
		FPredictProjectilePathResult PredictResult;
		if(Owner->GetTrackCharacterValues())
			Owner->TakeValuesFromClass();
		PredictParams.OverrideGravityZ = Owner->GetGravityScale() * World->GetGravityZ();
		PredictParams.LaunchVelocity = Owner->GetActorForwardVector() * Owner->GetSpeed();
		PredictParams.LaunchVelocity.Z += Owner->GetJumpZVelocity();
		PredictParams.StartLocation = Owner->GetActorLocation();
		PredictParams.MaxSimTime = Owner->GetSimTime();
		PredictParams.DrawDebugType = EDrawDebugTrace::None;
		UGameplayStatics::PredictProjectilePath(World, PredictParams, PredictResult);

		if(PredictResult.PathData.Num() < 2)
			return DSceneProxy;
		auto PreviousPoint = PredictResult.PathData[0];
		for(int i = 1; i < PredictResult.PathData.Num(); i++)
		{
			DSceneProxy->Lines.Emplace(PreviousPoint.Location, PredictResult.PathData[i].Location, FLinearColor::Yellow.ToFColor(true), 2.f);
			PreviousPoint = PredictResult.PathData[i];
		}
	}
	return DSceneProxy;
}

FBoxSphereBounds UJumpSimComp::CalcBounds(const FTransform& Transform) const
{
	FBoxSphereBounds::Builder BoundsBuilder;
	BoundsBuilder += Super::CalcBounds(Transform);
	BoundsBuilder += FSphere(GetComponentLocation(), 50.f);
	return BoundsBuilder;
}
