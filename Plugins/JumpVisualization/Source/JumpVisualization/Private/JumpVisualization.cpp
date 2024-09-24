// Copyright Epic Games, Inc. All Rights Reserved.

#include "JumpVisualization.h"

#include "DebugRenderSceneProxy.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "FCustomEditorViewportCommands.h"
#include "JumpVisActor.h"
#include "ToolMenus.h"
#include "Components/CapsuleComponent.h"
#include "Debug/DebugDrawService.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Serialization/BufferArchive.h"
#include "JumpVisActor.h"
#include "LevelEditor.h"
#include "SimulationCharacterMovementComponent.h"
#include "SLevelViewport.h"
#include "Containers/DirectoryTree.h"
#include "Framework/MultiBox/SToolBarComboButtonBlock.h"

#define LOCTEXT_NAMESPACE "FJumpVisualizationModule"

void FJumpVisualizationModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//FCustomEditorViewportCommands::Register();
	//BindCommands();
	
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FJumpVisualizationModule::RegisterMenuExtensions));
	FEditorDelegates::EndPIE.AddRaw(this, &FJumpVisualizationModule::PrintJumpLocations);
	RecordJumpsDelegate = FEditorDelegates::PostPIEStarted.AddRaw(this, &FJumpVisualizationModule::StartRecordingData);
	RecordJumpsDelegate = FEditorDelegates::ShutdownPIE.AddRaw(this, &FJumpVisualizationModule::OnEndPIE);
	
	//FEngineShowFlags::RegisterCustomShowFlag()

	ViewFlagName = TEXT("JumpVisualization");
	ViewFlagIndex = static_cast<uint32>(FEngineShowFlags::FindIndexByName(*ViewFlagName));
}

void FJumpVisualizationModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

TArray<TArray<FCapsuleLocation>> FJumpVisualizationModule::CalculateJumpLocation(const TArray<TArray<FCapsuleLocation>>& SessionJumps)
{
	if(SessionJumps.Num() == 0)
		return TArray<TArray<FCapsuleLocation>>();
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if(!World)
		World = GEditor->PlayWorld;

	if(!World)
		return TArray<TArray<FCapsuleLocation>>();
	
	USimulationCharacterMovementComponent SimChMovComp;
	TObjectPtr<ACharacter> SimCh = World->SpawnActor<ACharacter>();
	TObjectPtr<USceneComponent> SimSceneComp = World->SpawnActor<USceneComponent>();
	SimChMovComp.SetCharacterOwner(SimCh);
	SimChMovComp.SetUpdatedComponent(SimSceneComp);
	//TArray<FCapsuleLocation> JumpLocations;
	SimChMovComp.Velocity = SessionJumps[0][0].Velocity;
	SimChMovComp.AirControl = SessionJumps[0][0].AirControl;
	SimChMovComp.AirControlBoostMultiplier = SessionJumps[0][0].AirControlBoostMultiplier;
	SimChMovComp.AirControlBoostVelocityThreshold = SessionJumps[0][0].AirControlBoostVelocityThreshold;
	SimChMovComp.MaxSimulationIterations = SessionJumps[0][0].MaxSimulationIterations;
	SimChMovComp.MaxSimulationTimeStep = SessionJumps[0][0].MaxSimulationTimeStep;
	SimSceneComp->SetWorldLocation(SessionJumps[0][0].Location);
	SimSceneComp->SetWorldRotation(SessionJumps[0][0].Rotation);
	TArray<TArray<FCapsuleLocation>> Output;
	Output.Emplace(TArray<FCapsuleLocation>());
	for(int i = 0; i < 720; i++)
	{
		if(SessionJumps[0].Num() > i)
			SimChMovComp.SetAcceleration(SessionJumps[0][i].Acceleration);
		
		SimChMovComp.PhysFalling(0.016f, 8);
		FCapsuleLocation Location;
		Location.TopMiddle = SimChMovComp.GetActorLocation();
		Location.BottomMiddle = SimChMovComp.GetActorLocation();
		Location.TopMiddle.Z += SessionJumps[0][0].HalfCapsuleHeight;
		Location.BottomMiddle.Z -= SessionJumps[0][0].HalfCapsuleHeight;
		Location.Location = SimChMovComp.GetActorLocation();
		Output.Last().Emplace(Location);
	}
	
	return Output;
	
	//TArray<TArray<FCapsuleLocation>> NewJumpLocations;
	//FPredictProjectilePathParams PredictParams;
	//FPredictProjectilePathResult PredictResult;
	//if(Owner->TrackCharacterValues)
	//	Owner->TakeValuesFromClass();
	//PredictParams.OverrideGravityZ = Owner->GravityScale * World->GetGravityZ();
	//PredictParams.LaunchVelocity = Owner->GetActorForwardVector() * Owner->Speed;
	//PredictParams.LaunchVelocity.Z += Owner->JumpZVelocity;
	//PredictParams.StartLocation = Owner->GetActorLocation();
	//PredictParams.MaxSimTime = Owner->SimTime;
	//PredictParams.DrawDebugType = EDrawDebugTrace::None;
	//for (int32 i = 0; i < SessionJumps.Num(); i++)
	//{
	//	if(SessionJumps[i].Num() < 3)
	//		continue;
	//	FCapsuleLocation StartJump = SessionJumps[i][0];
//
	//	
	//}
	//SCOPE_CYCLE_COUNTER(STAT_CharPhysFalling);

	//if (CurrentCapsuleLocation.DeltaTime < UCharacterMovementComponent::MIN_TICK_TIME)
	//{
	//	return FVector(-1.f);
	//}
	//Acceleration - input * max acceleration
	//Air Control
	//MaxSimulationIterations
	//MaxSimulationTimeStep
	//Scene Component Location
	//Scene Component Quat
	//Root Motion (if used?)
	//Velocity
	//Falling Lateral Friction
	//bForceMaxAccel
	//AnalogInputModifier
	//bUseSeparateBrakingFriction
	//BrakingFriction
	//BrakingFrictionFactor
	//BrakingSubStepTime
	//CharacterOwner -> JumpForceTimeRemaining
	//bApplyGravityWhileJumping
	//CharacterOwner -> ResetJumpState()
		//bPressedJump = false;
		//bWasJumping = false;
		//JumpKeyHoldTime = 0.0f;
		//JumpForceTimeRemaining = 0.0f;

		//if (CharacterMovement && !CharacterMovement->IsFalling())
		//{
		//	JumpCurrentCount = 0;
		//	JumpCurrentCountPreJump = 0;
		//}
	// GetPhysicsVolume()->TerminalVelocity
	//FormerBaseVelocityDecayHalfLife
	//DecayingFormerBaseVelocity
	//NumJumpApexAttempts
	//MaxJumpApexAttemptsPerSimulation
	//SafeMoveUpdatedComponent()
	//
	//UCharacterMovementComponent SimulatingChMovComp;
	//USimulationCharacterMovementComponent SimulatingChMovComp;
	//SimulatingChMovComp
	//SimulatingChMovComp.SetAcceleration(CurrentCapsuleLocation.Acceleration);
	//SimulatingChMovComp.Velocity = CurrentCapsuleLocation.Velocity;
	//SimulatingChMovComp.AirControl = CurrentCapsuleLocation.AirControl;
	//SimulatingChMovComp.AirControlBoostMultiplier = CurrentCapsuleLocation.AirControlBoostMultiplier;
	//SimulatingChMovComp.AirControlBoostVelocityThreshold = CurrentCapsuleLocation.AirControlBoostVelocityThreshold;
	//SimulatingChMovComp.MaxSimulationIterations = CurrentCapsuleLocation.MaxSimulationIterations;
	//SimulatingChMovComp.MaxSimulationTimeStep = CurrentCapsuleLocation.MaxSimulationTimeStep;
	//
	//SimulatingChMovComp.PhysFalling(0.016f, 8);
	
	//FVector FallAcceleration;// = SimulatingChMovComp.GetFallingLateralAcceleration(deltaTime);
	//{
	//	const FVector GravityRelativeAcceleration = SimulatingChMovComp.RotateWorldToGravity(CurrentCapsuleLocation.Acceleration);
	//	FallAcceleration = SimulatingChMovComp.RotateGravityToWorld(FVector(GravityRelativeAcceleration.X, GravityRelativeAcceleration.Y, 0.f));
//
	//	// bound acceleration, falling object has minimal ability to impact acceleration
	//	if (GravityRelativeAcceleration.SizeSquared2D() > 0.f)
	//	{
	//		FallAcceleration =  SimulatingChMovComp.GetAirControl(CurrentCapsuleLocation.DeltaTime, CurrentCapsuleLocation.AirControl, FallAcceleration);
	//		FallAcceleration = FallAcceleration.GetClampedToMaxSize(CurrentCapsuleLocation.MaxAcceleration);
	//	}
	//}
	////Check GravityToWorldTransform and WorldToGravityTransform
	//const FVector GravityRelativeFallAcceleration = SimulatingChMovComp.RotateWorldToGravity(FallAcceleration);
	//FallAcceleration = SimulatingChMovComp.RotateGravityToWorld(FVector(GravityRelativeFallAcceleration.X, GravityRelativeFallAcceleration.Y, 0));
	//const bool bHasLimitedAirControl = SimulatingChMovComp.ShouldLimitAirControl(CurrentCapsuleLocation.DeltaTime, FallAcceleration);
//
	//float remainingTime = deltaTime;
	//while( (remainingTime >= MIN_TICK_TIME) && (Iterations < CurrentCapsuleLocation.MaxSimulationIterations) )
	//{
	//	Iterations++;
	//	float timeTick = SimulatingChMovComp.GetSimulationTimeStep(remainingTime, Iterations);
	//	remainingTime -= timeTick;
	//	
	//	const FVector OldLocation = CurrentCapsuleLocation.Location;
	//	const FQuat PawnRotation = CurrentCapsuleLocation.Rotation;
	//	SimulatingChMovComp.bJustTeleported = false;
//
	//	const FVector OldVelocityWithRootMotion = CurrentCapsuleLocation.Velocity;
//
	//	//If root motion?
	//	//RestorePreAdditiveRootMotionVelocity();
//
	//	const FVector OldVelocity = CurrentCapsuleLocation.Velocity;
//
	//	// Apply input
	//	const float MaxDecel = CurrentCapsuleLocation.MaxFlyDeceleration;
	//	//if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	//	{
	//		// Compute Velocity
	//		{
	//			// Acceleration = FallAcceleration for CalcVelocity(), but we restore it after using it.
	//			TGuardValue<FVector> RestoreAcceleration(Acceleration, FallAcceleration);
	//			//if (HasCustomGravity())
	//			//{
	//			//	Velocity = FVector::VectorPlaneProject(Velocity, RotateGravityToWorld(FVector::UpVector));
	//			//	const FVector GravityRelativeOffset = OldVelocity - Velocity;
	//			//	SimulatingChMovComp.CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
	//			//	Velocity += GravityRelativeOffset;
	//			//}
	//			//else
	//			{
	//				SimulatingChMovComp.Velocity.Z = 0.f;
	//				SimulatingChMovComp.CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
	//				Velocity.Z = OldVelocity.Z;
	//			}
	//		}
	//	}
//
	//	// Compute current gravity
	//	const FVector Gravity = -GetGravityDirection() * GetGravityZ();
	//	float GravityTime = timeTick;
//
	//	// If jump is providing force, gravity may be affected.
	//	bool bEndingJumpForce = false;
	//	if (CharacterOwner->JumpForceTimeRemaining > 0.0f)
	//	{
	//		// Consume some of the force time. Only the remaining time (if any) is affected by gravity when bApplyGravityWhileJumping=false.
	//		const float JumpForceTime = FMath::Min(CharacterOwner->JumpForceTimeRemaining, timeTick);
	//		GravityTime = bApplyGravityWhileJumping ? timeTick : FMath::Max(0.0f, timeTick - JumpForceTime);
	//		
	//		// Update Character state
	//		CharacterOwner->JumpForceTimeRemaining -= JumpForceTime;
	//		if (CharacterOwner->JumpForceTimeRemaining <= 0.0f)
	//		{
	//			CharacterOwner->ResetJumpState();
	//			bEndingJumpForce = true;
	//		}
	//	}
//
	//	// Apply gravity
	//	Velocity = NewFallVelocity(Velocity, Gravity, GravityTime);
//
	//	//UE_LOG(LogCharacterMovement, Log, TEXT("dt=(%.6f) OldLocation=(%s) OldVelocity=(%s) OldVelocityWithRootMotion=(%s) NewVelocity=(%s)"), timeTick, *(UpdatedComponent->GetComponentLocation()).ToString(), *OldVelocity.ToString(), *OldVelocityWithRootMotion.ToString(), *Velocity.ToString());
	//	ApplyRootMotionToVelocity(timeTick);
	//	DecayFormerBaseVelocity(timeTick);
//
	//	// See if we need to sub-step to exactly reach the apex. This is important for avoiding "cutting off the top" of the trajectory as framerate varies.
	//	const FVector GravityRelativeOldVelocityWithRootMotion = RotateWorldToGravity(OldVelocityWithRootMotion);
	//	if (CharacterMovementCVars::ForceJumpPeakSubstep && GravityRelativeOldVelocityWithRootMotion.Z > 0.f && RotateWorldToGravity(Velocity).Z <= 0.f && NumJumpApexAttempts < MaxJumpApexAttemptsPerSimulation)
	//	{
	//		const FVector DerivedAccel = (Velocity - OldVelocityWithRootMotion) / timeTick;
	//		const FVector GravityRelativeDerivedAccel = RotateWorldToGravity(DerivedAccel);
	//		if (!FMath::IsNearlyZero(GravityRelativeDerivedAccel.Z))
	//		{
	//			const float TimeToApex = -GravityRelativeOldVelocityWithRootMotion.Z / GravityRelativeDerivedAccel.Z;
	//			
	//			// The time-to-apex calculation should be precise, and we want to avoid adding a substep when we are basically already at the apex from the previous iteration's work.
	//			const float ApexTimeMinimum = 0.0001f;
	//			if (TimeToApex >= ApexTimeMinimum && TimeToApex < timeTick)
	//			{
	//				const FVector ApexVelocity = OldVelocityWithRootMotion + (DerivedAccel * TimeToApex);
	//				if (HasCustomGravity())
	//				{
	//					const FVector GravityRelativeApexVelocity = RotateWorldToGravity(ApexVelocity);
	//					Velocity = RotateGravityToWorld(FVector(GravityRelativeApexVelocity.X, GravityRelativeApexVelocity.Y, 0)); // Should be nearly zero anyway, but this makes apex notifications consistent.
	//				}
	//				else
	//				{
	//					Velocity = ApexVelocity;
	//					Velocity.Z = 0.f; // Should be nearly zero anyway, but this makes apex notifications consistent.
	//				}
	//				
	//				// We only want to move the amount of time it takes to reach the apex, and refund the unused time for next iteration.
	//				const float TimeToRefund = (timeTick - TimeToApex);
//
	//				remainingTime += TimeToRefund;
	//				timeTick = TimeToApex;
	//				Iterations--;
	//				NumJumpApexAttempts++;
//
	//				// Refund time to any active Root Motion Sources as well
	//				for (TSharedPtr<FRootMotionSource> RootMotionSource : CurrentRootMotion.RootMotionSources)
	//				{
	//					const float RewoundRMSTime = FMath::Max(0.0f, RootMotionSource->GetTime() - TimeToRefund);
	//					RootMotionSource->SetTime(RewoundRMSTime);
	//				}
	//			}
	//		}
	//	}
//
	//	if (bNotifyApex && (RotateWorldToGravity(Velocity).Z < 0.f))
	//	{
	//		// Just passed jump apex since now going down
	//		bNotifyApex = false;
	//		NotifyJumpApex();
	//	}
//
	//	// Compute change in position (using midpoint integration method).
	//	FVector Adjusted = 0.5f * (OldVelocityWithRootMotion + Velocity) * timeTick;
	//	
	//	// Special handling if ending the jump force where we didn't apply gravity during the jump.
	//	if (bEndingJumpForce && !bApplyGravityWhileJumping)
	//	{
	//		// We had a portion of the time at constant speed then a portion with acceleration due to gravity.
	//		// Account for that here with a more correct change in position.
	//		const float NonGravityTime = FMath::Max(0.f, timeTick - GravityTime);
	//		Adjusted = (OldVelocityWithRootMotion * NonGravityTime) + (0.5f*(OldVelocityWithRootMotion + Velocity) * GravityTime);
	//	}
//
	//	// Move
	//	FHitResult Hit(1.f);
	//	SafeMoveUpdatedComponent( Adjusted, PawnRotation, true, Hit);
	//	
	//	if (!HasValidData())
	//	{
	//		return;
	//	}
	//	
	//	float LastMoveTimeSlice = timeTick;
	//	float subTimeTickRemaining = timeTick * (1.f - Hit.Time);
	//	
	//	/*if ( IsSwimming() ) //just entered water
	//	{
	//		remainingTime += subTimeTickRemaining;
	//		StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
	//		return;
	//	}*/
	//	/*else if ( Hit.bBlockingHit )
	//	{
	//		if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
	//		{
	//			remainingTime += subTimeTickRemaining;
	//			ProcessLanded(Hit, remainingTime, Iterations);
	//			return;
	//		}
	//		else
	//		{
	//			// Compute impact deflection based on final velocity, not integration step.
	//			// This allows us to compute a new velocity from the deflected vector, and ensures the full gravity effect is included in the slide result.
	//			Adjusted = Velocity * timeTick;
//
	//			// See if we can convert a normally invalid landing spot (based on the hit result) to a usable one.
	//			if (!Hit.bStartPenetrating && ShouldCheckForValidLandingSpot(timeTick, Adjusted, Hit))
	//			{
	//				const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
	//				FFindFloorResult FloorResult;
	//				FindFloor(PawnLocation, FloorResult, false);
	//				if (FloorResult.IsWalkableFloor() && IsValidLandingSpot(PawnLocation, FloorResult.HitResult))
	//				{
	//					remainingTime += subTimeTickRemaining;
	//					ProcessLanded(FloorResult.HitResult, remainingTime, Iterations);
	//					return;
	//				}
	//			}
//
	//			HandleImpact(Hit, LastMoveTimeSlice, Adjusted);
	//			
	//			// If we've changed physics mode, abort.
	//			if (!HasValidData() || !IsFalling())
	//			{
	//				return;
	//			}
//
	//			// Limit air control based on what we hit.
	//			// We moved to the impact point using air control, but may want to deflect from there based on a limited air control acceleration.
	//			FVector VelocityNoAirControl = OldVelocity;
	//			FVector AirControlAccel = Acceleration;
	//			if (bHasLimitedAirControl)
	//			{
	//				// Compute VelocityNoAirControl
	//				{
	//					// Find velocity *without* acceleration.
	//					TGuardValue<FVector> RestoreAcceleration(Acceleration, FVector::ZeroVector);
	//					TGuardValue<FVector> RestoreVelocity(Velocity, OldVelocity);
	//					if (HasCustomGravity())
	//					{
	//						Velocity = FVector::VectorPlaneProject(Velocity, RotateGravityToWorld(FVector::UpVector));
	//						const FVector GravityRelativeOffset = OldVelocity - Velocity;
	//						CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
	//						VelocityNoAirControl = Velocity + GravityRelativeOffset;
	//					}
	//					else
	//					{
	//						Velocity.Z = 0.f;
	//						CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
	//						VelocityNoAirControl = FVector(Velocity.X, Velocity.Y, OldVelocity.Z);
	//					}
	//					
	//					VelocityNoAirControl = NewFallVelocity(VelocityNoAirControl, Gravity, GravityTime);
	//				}
//
	//				const bool bCheckLandingSpot = false; // we already checked above.
	//				AirControlAccel = (Velocity - VelocityNoAirControl) / timeTick;
	//				const FVector AirControlDeltaV = LimitAirControl(LastMoveTimeSlice, AirControlAccel, Hit, bCheckLandingSpot) * LastMoveTimeSlice;
	//				Adjusted = (VelocityNoAirControl + AirControlDeltaV) * LastMoveTimeSlice;
	//			}
//
	//			const FVector OldHitNormal = Hit.Normal;
	//			const FVector OldHitImpactNormal = Hit.ImpactNormal;				
	//			FVector Delta = ComputeSlideVector(Adjusted, 1.f - Hit.Time, OldHitNormal, Hit);
//
	//			// Compute velocity after deflection (only gravity component for RootMotion)
	//			const UPrimitiveComponent* HitComponent = Hit.GetComponent();
	//			//if (CharacterMovementCVars::UseTargetVelocityOnImpact && !Velocity.IsNearlyZero() && MovementBaseUtility::IsSimulatedBase(HitComponent))
	//			//{
	//			//	const FVector ContactVelocity = MovementBaseUtility::GetMovementBaseVelocity(HitComponent, NAME_None) + MovementBaseUtility::GetMovementBaseTangentialVelocity(HitComponent, NAME_None, Hit.ImpactPoint);
	//			//	const FVector NewVelocity = Velocity - Hit.ImpactNormal * FVector::DotProduct(Velocity - ContactVelocity, Hit.ImpactNormal);
	//			//	Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? FVector(Velocity.X, Velocity.Y, NewVelocity.Z) : NewVelocity;
	//			//}
	//			//else if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && !bJustTeleported)
	//			//{
	//			//	const FVector NewVelocity = (Delta / subTimeTickRemaining);
	//			//	Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? FVector(Velocity.X, Velocity.Y, NewVelocity.Z) : NewVelocity;
	//			//}
//
	//			//if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && (Delta | Adjusted) > 0.f)
	//			//{
	//				// Move in deflected direction.
	//			//	SafeMoveUpdatedComponent( Delta, PawnRotation, true, Hit);
	//				
	//				//if (Hit.bBlockingHit)
	//				//{
	//				//	// hit second wall
	//				//	LastMoveTimeSlice = subTimeTickRemaining;
	//				//	subTimeTickRemaining = subTimeTickRemaining * (1.f - Hit.Time);
////
	//				//	if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
	//				//	{
	//				//		remainingTime += subTimeTickRemaining;
	//				//		ProcessLanded(Hit, remainingTime, Iterations);
	//				//		return;
	//				//	}
////
	//				//	HandleImpact(Hit, LastMoveTimeSlice, Delta);
////
	//				//	// If we've changed physics mode, abort.
	//				//	if (!HasValidData() || !IsFalling())
	//				//	{
	//				//		return;
	//				//	}
////
	//				//	// Act as if there was no air control on the last move when computing new deflection.
	//				//	if (bHasLimitedAirControl && RotateWorldToGravity(Hit.Normal).Z > CharacterMovementConstants::VERTICAL_SLOPE_NORMAL_Z)
	//				//	{
	//				//		const FVector LastMoveNoAirControl = VelocityNoAirControl * LastMoveTimeSlice;
	//				//		Delta = ComputeSlideVector(LastMoveNoAirControl, 1.f, OldHitNormal, Hit);
	//				//	}
////
	//				//	FVector PreTwoWallDelta = Delta;
	//				//	TwoWallAdjust(Delta, Hit, OldHitNormal);
////
	//				//	// Limit air control, but allow a slide along the second wall.
	//				//	if (bHasLimitedAirControl)
	//				//	{
	//				//		const bool bCheckLandingSpot = false; // we already checked above.
	//				//		const FVector AirControlDeltaV = LimitAirControl(subTimeTickRemaining, AirControlAccel, Hit, bCheckLandingSpot) * subTimeTickRemaining;
////
	//				//		// Only allow if not back in to first wall
	//				//		if (FVector::DotProduct(AirControlDeltaV, OldHitNormal) > 0.f)
	//				//		{
	//				//			Delta += (AirControlDeltaV * subTimeTickRemaining);
	//				//		}
	//				//	}
////
	//				//	// Compute velocity after deflection (only gravity component for RootMotion)
	//				//	if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && !bJustTeleported)
	//				//	{
	//				//		const FVector NewVelocity = (Delta / subTimeTickRemaining);
	//				//		Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? FVector(Velocity.X, Velocity.Y, NewVelocity.Z) : NewVelocity;
	//				//	}
////
	//				//	// bDitch=true means that pawn is straddling two slopes, neither of which it can stand on
	//				//	bool bDitch = ( (RotateWorldToGravity(OldHitImpactNormal).Z > 0.f) && (RotateWorldToGravity(Hit.ImpactNormal).Z > 0.f) && (FMath::Abs(Delta.Z) <= UE_KINDA_SMALL_NUMBER) && ((Hit.ImpactNormal | OldHitImpactNormal) < 0.f) );
	//				//	SafeMoveUpdatedComponent( Delta, PawnRotation, true, Hit);
	//				//	if ( Hit.Time == 0.f )
	//				//	{
	//				//		// if we are stuck then try to side step
	//				//		FVector SideDelta = (OldHitNormal + Hit.ImpactNormal).GetSafeNormal2D();
	//				//		if ( SideDelta.IsNearlyZero() )
	//				//		{
	//				//			SideDelta = FVector(OldHitNormal.Y, -OldHitNormal.X, 0).GetSafeNormal();
	//				//		}
	//				//		SafeMoveUpdatedComponent( SideDelta, PawnRotation, true, Hit);
	//				//	}
	//				//		
	//				//	if ( bDitch || IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit) || Hit.Time == 0.f  )
	//				//	{
	//				//		remainingTime = 0.f;
	//				//		ProcessLanded(Hit, remainingTime, Iterations);
	//				//		return;
	//				//	}
	//				//	else if (GetPerchRadiusThreshold() > 0.f && Hit.Time == 1.f && RotateWorldToGravity(OldHitImpactNormal).Z >= WalkableFloorZ)
	//				//	{
	//				//		// We might be in a virtual 'ditch' within our perch radius. This is rare.
	//				//		const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
	//				//		const float ZMovedDist = FMath::Abs(RotateWorldToGravity(PawnLocation - OldLocation).Z);
	//				//		const float MovedDist2DSq = FVector::VectorPlaneProject(PawnLocation - OldLocation, RotateGravityToWorld(FVector::UpVector)).Size2D();
	//				//		if (ZMovedDist <= 0.2f * timeTick && MovedDist2DSq <= 4.f * timeTick)
	//				//		{
	//				//			FVector GravityRelativeVelocity = RotateWorldToGravity(Velocity);
	//				//			GravityRelativeVelocity.X += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
	//				//			GravityRelativeVelocity.Y += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
	//				//			GravityRelativeVelocity.Z = FMath::Max<float>(JumpZVelocity * 0.25f, 1.f);
	//				//			Velocity = RotateGravityToWorld(GravityRelativeVelocity);
	//				//			Delta = Velocity * timeTick;
	//				//			SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
	//				//		}
	//				//	}
	//				//}
	//			}
	//		}
	//	}*/
//
	//	FVector GravityRelativeVelocity = RotateWorldToGravity(Velocity);
	//	if (GravityRelativeVelocity.SizeSquared2D() <= UE_KINDA_SMALL_NUMBER * 10.f)
	//	{
	//		GravityRelativeVelocity.X = 0.f;
	//		GravityRelativeVelocity.Y = 0.f;
	//		Velocity = RotateGravityToWorld(GravityRelativeVelocity);
	//	}
	//}
}

void FJumpVisualizationModule::ToggleJumpVisualization()
{
	IsJumpVisible = !IsJumpVisible;
	if(IsJumpVisible)
	{
		PrintJumpLocations(false);
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
		if (LevelEditor.IsValid())
		{
			TArray<TSharedPtr<SLevelViewport>> Viewports = LevelEditor->GetViewports();
			for (const TSharedPtr<SLevelViewport>& ViewportWindow : Viewports)
			{
				if (ViewportWindow.IsValid())
				{
					FEditorViewportClient& Viewport = ViewportWindow->GetAssetViewportClient();
					Viewport.EngineShowFlags.SetSingleFlag(ViewFlagIndex, true);
				}
			}
		}
	}
	else
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
		if (LevelEditor.IsValid())
		{
			TArray<TSharedPtr<SLevelViewport>> Viewports = LevelEditor->GetViewports();
			for (const TSharedPtr<SLevelViewport>& ViewportWindow : Viewports)
			{
				if (ViewportWindow.IsValid())
				{
					FEditorViewportClient& Viewport = ViewportWindow->GetAssetViewportClient();
					Viewport.EngineShowFlags.SetSingleFlag(ViewFlagIndex, false);
				}
			}
		}
		//FEditorDelegates::PostPIEStarted.Remove(RecordJumpsDelegate);
	}
	//PawnRef->
}

bool FJumpVisualizationModule::IsJumpVisualizationVisible() const
{
	return IsJumpVisible;
}

void FJumpVisualizationModule::BindCommands()
{
	const FCustomEditorViewportCommands& Commands = FCustomEditorViewportCommands::Get();
	CommandList = MakeShared<FUICommandList>();
	CommandList->MapAction(
	  	Commands.JumpVisualization,
	 	FExecuteAction::CreateRaw(this, &FJumpVisualizationModule::ToggleJumpVisualization),
	  	FCanExecuteAction(),
	  	FIsActionChecked::CreateRaw(this, &FJumpVisualizationModule::IsJumpVisualizationVisible));
}

void FJumpVisualizationModule::RegisterMenuExtensions()
{
	//BindCommands();
	FToolMenuOwnerScoped OwnerScoped(this);
	UToolMenu* ShowMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelViewportToolbar.Show");
	FToolMenuSection& ShowMenuSection = ShowMenu->FindOrAddSection("CustomLevelViewportEditorShow");
	ShowMenuSection.AddSeparator(FName("Custom Level Editor Tools"));
	FUIAction Action (FExecuteAction::CreateRaw(this, &FJumpVisualizationModule::ToggleJumpVisualization),
		  FCanExecuteAction(),
		  FIsActionChecked::CreateRaw(this, &FJumpVisualizationModule::IsJumpVisualizationVisible));
	ShowMenuSection.AddMenuEntry(FName("JumpVisualizer"), FText::FromString("Jump Visualizer"), FText::FromString("Visualize Jumps From Previous Play"), FSlateIcon(), Action, EUserInterfaceActionType::ToggleButton);
	TSharedPtr<SComboButton> ComboButton = SNew(SComboButton).AccessibleText(LOCTEXT("Combo", ""));
	
	//ShowMenuSection.AddEntry(FToolMenuEntry::InitWidget("ComboButton", ComboButton.ToSharedRef(), FText::GetEmpty()));
	//ShowMenuSection.AddSubMenu()
	//ShowMenuSection.AddMenuEntry(FName("JumpVisualizer"), FText::FromString("Jump Visualizer"), FText::FromString("Visualize Jumps From Previous Play"), FSlateIcon(), Action, EUserInterfaceActionType::);
}

void FJumpVisualizationModule::StartRecordingData(bool IsSimulating)
{
	JumpLocations.Empty();
	for(auto& Pair : ResourceData)
		Pair.Value.Empty();
	for(TActorIterator<AJumpVisActor> ActorIt(GEditor->PlayWorld); ActorIt; ++ActorIt)
	{
		JumpVisActor = *ActorIt;
	}

	if(!JumpVisActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find jump visualization actor in world. Make sure there is one!"));
		return;
	}

	CheckJumpDelegate = FCoreDelegates::OnEndFrame.AddRaw(this, &FJumpVisualizationModule::CheckPlayerData);
	//CheckResourceDelegate = FCoreDelegates::OnEndFrame.AddRaw(this, &FJumpVisualizationModule::CollectResourceData);
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateRaw(this, &FJumpVisualizationModule::CollectResourceData);
	GEditor->GetTimerManager()->SetTimer(CollectResourceDataTimer, TimerDelegate, 0.5f, true, 0.f);

}

void FJumpVisualizationModule::CheckPlayerData()
{
	APlayerController* PlayerControllerRef = UGameplayStatics::GetPlayerController(GEditor->PlayWorld, 0);
	if(!PlayerControllerRef)
		return;
	APawn* PawnRef = PlayerControllerRef->GetPawn();
	ACharacter* CharacterRef = Cast<ACharacter>(PawnRef);
	if(!CharacterRef)
		return;
	
	if(CharacterRef->bWasJumping)
	{
		FCoreDelegates::OnEndFrame.Remove(CheckJumpDelegate);
		JumpLocations.Add(TArray<FCapsuleLocation>());
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateRaw(this, &FJumpVisualizationModule::CollectJumpData, CharacterRef);
		GEditor->GetTimerManager()->SetTimer(CollectJumpDataTimer, TimerDelegate, 0.1f, true, 0.f);
	}
}

void FJumpVisualizationModule::CollectJumpData(ACharacter* Character)
{
	if(!Character)
		return;
	UCharacterMovementComponent* MovementRef = Character->GetCharacterMovement();
	UCapsuleComponent* CapsuleRef = Character->GetCapsuleComponent();
	if(!MovementRef || !CapsuleRef)
		return;
	FCapsuleLocation Location;
	float Radius = 0.f;
	float HalfHeight = 0.f;
	CapsuleRef->GetScaledCapsuleSize(Radius, HalfHeight);
	Location.JumpZVelocity = MovementRef->JumpZVelocity;
	Location.Velocity = MovementRef->Velocity;
	Location.GravityScale = MovementRef->GravityScale;
	Location.Location = CapsuleRef->GetComponentLocation();
	Location.HalfCapsuleHeight = HalfHeight;
	Location.TopMiddle = Location.BottomMiddle = CapsuleRef->GetComponentLocation();
	Location.TopMiddle.Z += HalfHeight;
	Location.BottomMiddle.Z -= HalfHeight;
	Location.Time = FDateTime::Now();
	Location.Velocity = Character->GetVelocity();
	Location.Speed = Character->GetVelocity().Size();
	Location.GravityScale = MovementRef->GravityScale;
	Location.JumpZVelocity = MovementRef->JumpZVelocity;
	JumpLocations.Last().Add(Location);
	if(MovementRef && !MovementRef->IsFalling())
	{
		GEditor->GetTimerManager()->ClearTimer(CollectJumpDataTimer);
		CheckJumpDelegate = FCoreDelegates::OnEndFrame.AddRaw(this, &FJumpVisualizationModule::CheckPlayerData);
	}
}

void FJumpVisualizationModule::CollectResourceData()
{
	APlayerController* PlayerControllerRef = UGameplayStatics::GetPlayerController(GEditor->PlayWorld, 0);
	if(!PlayerControllerRef)
		return;
	APawn* PawnRef = PlayerControllerRef->GetPawn();
	ACharacter* CharacterRef = Cast<ACharacter>(PawnRef);
	if(!CharacterRef)
		return;
	UCharacterMovementComponent* MovementRef = CharacterRef->GetCharacterMovement();
	UCapsuleComponent* CapsuleRef = CharacterRef->GetCapsuleComponent();
	if(!MovementRef || !CapsuleRef)
		return;
	float Radius = 0.f;
	float HalfHeight = 0.f;
	CapsuleRef->GetScaledCapsuleSize(Radius, HalfHeight);
	FVector Location =  CapsuleRef->GetComponentLocation();
	FDateTime Time = FDateTime::Now();
	for(auto& Resource : ResourceData)
	{
		Resource.Value.Emplace(FResourceData(*Resource.Key, Location, Time));
	}
}

void FJumpVisualizationModule::PrintJumpLocations(bool IsSimulating)
{
	FCoreDelegates::OnEndFrame.Remove(CheckJumpDelegate);
	GEditor->GetTimerManager()->ClearTimer(CollectJumpDataTimer);
	if(!JumpVisActor)
	{
		return;
	}
}

void FJumpVisualizationModule::OnEndPIE(bool IsSimulating)
{
	FBufferArchive Archive;
	Archive << JumpLocations;
	FDateTime CurrentTime = FDateTime::Now();
	FString NewFileName = TEXT("JumpDataFile");
	NewFileName.Append(CurrentTime.ToString());
	NewFileName.Append(TEXT(".dat"));
	FString FullPath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/"), NewFileName);
	TArray<FString> FileNames;
	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	while(FileNames.Num() > 99)
	{
		bool bFoundFile = false;
		FString OldestFile = GetOldestFile(bFoundFile);
		if(!bFoundFile)
		{
			break;
		}
		IFileManager::Get().Delete(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/"), OldestFile), false, true);
	}
	
	if(FFileHelper::SaveArrayToFile(Archive, *FullPath))
	{
		Archive.FlushCache();
		Archive.Empty();
	}
}

FString FJumpVisualizationModule::GetOldestFile(bool& bFoundFile)
{
	TArray<FString> FileNames;
	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	FString OldestFile = TEXT("");
	FDateTime OldestFileTimestamp = FDateTime::MaxValue();
	for(int i = 0; i < FileNames.Num(); i++)
	{
		//18
		FString FileName = FileNames[i];
		FileName.RemoveFromEnd(".dat");
		FString TimeStampString = FileName.RightChop(18);

		FDateTime Timestamp;
		FDateTime::Parse(TimeStampString, Timestamp);

		if(Timestamp < OldestFileTimestamp)
		{
			OldestFileTimestamp = Timestamp;
			OldestFile = FileNames[i];
			bFoundFile = true;
		}
	}
	return OldestFile;
}

FString FJumpVisualizationModule::GetNewestFile(bool& bFoundFile)
{
	TArray<FString> FileNames;
	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	FString NewestFile = TEXT("");
	FDateTime NewestFileTimestamp = FDateTime::MinValue();
	for(int i = 0; i < FileNames.Num(); i++)
	{
		//18
		FString FileName = FileNames[i];
		FileName.RemoveFromEnd(".dat");
		FileName.RightInline(19);
		
		FDateTime Timestamp;
		FDateTime::Parse(FileName, Timestamp);

		if(Timestamp > NewestFileTimestamp)
		{
			NewestFileTimestamp = Timestamp;
			NewestFile = FileNames[i];
			bFoundFile = true;
		}
	}
	return NewestFile;
}

FString FJumpVisualizationModule::GetNFile(bool& bFoundFile, int N)
{
	TArray<FString> FileNames;
	TMap<FDateTime, FString> FileMap;
	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	FString NewestFile = TEXT("");
	for(int i = 0; i < FileNames.Num(); i++)
	{
		//18
		FString FileName = FileNames[i];
		FileName.RemoveFromEnd(".dat");
		FileName.RightInline(19);
		
		FDateTime Timestamp;
		FDateTime::Parse(FileName, Timestamp);

		FileMap.Add(Timestamp, FileNames[i]);
	}
	
	TArray<FDateTime> Keys;
	FileMap.GenerateKeyArray(Keys);
	N = Keys.Num() - N;
	for(int i = Keys.Num() - 1; i >= 0; i--)
	{
		if(i == N)
		{
			bFoundFile = true;
        	return FileMap[Keys[i]];
		}
	}
	
	bFoundFile = false;
	return "";
}

int FJumpVisualizationModule::GetAmountOfFiles()
{	
	TArray<FString> FileNames;
 	IFileManager::Get().FindFiles(FileNames, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("JumpData/")), TEXT(".dat"));
	return FileNames.Num();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FJumpVisualizationModule, JumpVisualization)