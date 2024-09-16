#include "FJumpDebugSceneProxy.h"

FJumpDebugSceneProxy::FJumpDebugSceneProxy(const UPrimitiveComponent* InComponent)
	: FDebugRenderSceneProxy(InComponent)
{
	DrawType = EDrawType::WireMesh;
	DrawAlpha = 1;

	ViewFlagName = TEXT("JumpVisualization");
	ViewFlagIndex = static_cast<uint32>(FEngineShowFlags::FindIndexByName(*ViewFlagName));
}

FPrimitiveViewRelevance FJumpDebugSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance ViewRelevance;
	
	ViewRelevance.bDrawRelevance = ViewFlagIndex != INDEX_NONE && View->Family->EngineShowFlags.GetSingleFlag(ViewFlagIndex);
	ViewRelevance.bSeparateTranslucency = ViewRelevance.bNormalTranslucency = true;
	ViewRelevance.bDynamicRelevance = true;
	ViewRelevance.bShadowRelevance = false;
	
	return ViewRelevance;
}

void FJumpDebugSceneProxy::GetDynamicMeshElementsForView(const FSceneView* View, const int32 ViewIndex,
	const FSceneViewFamily& ViewFamily, const uint32 VisibilityMap, FMeshElementCollector& Collector,
	FMaterialCache& DefaultMaterialCache, FMaterialCache& SolidMeshMaterialCache) const
{
	FDebugRenderSceneProxy::GetDynamicMeshElementsForView(View, ViewIndex, ViewFamily, VisibilityMap, Collector,
	                                                      DefaultMaterialCache, SolidMeshMaterialCache);
}
