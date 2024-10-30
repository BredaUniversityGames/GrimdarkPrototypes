#pragma once
#include "DebugRenderSceneProxy.h"

class FJumpDebugSceneProxy final : public FDebugRenderSceneProxy
{
public:
	FJumpDebugSceneProxy(const UPrimitiveComponent* InComponent);

protected:
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	virtual void GetDynamicMeshElementsForView(const FSceneView* View, const int32 ViewIndex, const FSceneViewFamily& ViewFamily, const uint32 VisibilityMap, FMeshElementCollector& Collector, FMaterialCache& DefaultMaterialCache, FMaterialCache& SolidMeshMaterialCache) const override;
	virtual uint32 GetMemoryFootprint(void) const override { return sizeof(this) + GetAllocatedSize(); }
};
