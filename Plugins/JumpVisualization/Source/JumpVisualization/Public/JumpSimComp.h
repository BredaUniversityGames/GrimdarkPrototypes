#pragma once
#include "Debug/DebugDrawComponent.h"

#include "JumpSimComp.generated.h"

UCLASS()
class JUMPVISUALIZATION_API UJumpSimComp : public UDebugDrawComponent
{
	GENERATED_BODY()
	
public:
	UJumpSimComp();
	virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& Transform) const override;
	virtual bool ShouldRecreateProxyOnUpdateTransform() const override {return true;};
};
