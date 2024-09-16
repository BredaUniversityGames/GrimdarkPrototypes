#include "JumpVisComp.h"

#include "FJumpDebugSceneProxy.h"
#include "JumpVisActor.h"
#include "JumpVisData.h"

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
	Locations.Emplace(FVector(0.f));
	Locations.Emplace(FVector(0.f, 0.f, 1000.f));
}

FDebugRenderSceneProxy* UJumpVisComp::CreateDebugSceneProxy()
{
	FJumpDebugSceneProxy* DSceneProxy = new FJumpDebugSceneProxy(this);
	if(const AJumpVisActor* Actor = Cast<AJumpVisActor>(GetOwner()))
	{
		DSceneProxy->Lines.Emplace(Locations[0], Locations[1], FColor::Red, 5.f);
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




