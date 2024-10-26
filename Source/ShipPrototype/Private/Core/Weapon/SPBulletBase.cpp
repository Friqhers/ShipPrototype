// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Weapon/SPBulletBase.h"

#include "GameFramework/DefaultPawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/RadialForceComponent.h"

#define BULLET_TRACE_CHANNEL ECC_GameTraceChannel1
const float ASPBulletBase::MIN_TICK_TIME_BULLET = 1e-6f;
// Sets default values
ASPBulletBase::ASPBulletBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
	RootComponent = SceneRootComponent;

	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
	RadialForceComponent->SetupAttachment(RootComponent);
	
	bHasPendingDestroy = false;
}

// Called when the game starts or when spawned
void ASPBulletBase::BeginPlay()
{
	Super::BeginPlay();
	OwnerPawn = Cast<APawn>(GetOwner());

	if(!OwnerPawn)
	{
		StopSimulation();
		return;
	}
	

	//
	FVector Direction = GetActorRotation().Vector();
	Direction.Normalize();

	
	

	CurrentVelocity = Direction * InitialVelocity;
	
	
	CurrentLocation = GetActorLocation();

	AdjustedGravity = -FMath::Abs(UPhysicsSettings::Get()->DefaultGravityZ * GravityScale);

	IgnoredActors = GetIgnoredActors();

	if(DamageTypeToApply != EDamageTypeCustom::RadialDamage)
	{
		RadialForceComponent->DestroyComponent();
	}

	GetWorldTimerManager().SetTimer(TimerHandle_Simulation, this, &ASPBulletBase::StopSimulation, MaxSimTime, false);
}

// Called every frame
void ASPBulletBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(bHasPendingDestroy) return;
	if(DeltaTime < MIN_TICK_TIME_BULLET) return;

	float remainingTime = DeltaTime;
	int32 Iterations = 0;
	while ( (remainingTime >= MIN_TICK_TIME_BULLET) && (Iterations < MaxSimulationIterations))
	{
		Iterations++;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		//***********************************************//
		CurrentVelocity.Z += AdjustedGravity * timeTick; 
	
		// Compute move parameters
		const FVector Delta = timeTick * CurrentVelocity; // dx = v * dt

		FVector TargetLocation = CurrentLocation + Delta;
		
		

		EDrawDebugTrace::Type DrawDebugType = ShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
		FHitResult HitResult;
		// if bullet radius is almost zero or negative, we can just use line trace to save performance
		if(FMath::IsNearlyZero(BulletRadius, 0.01) || BulletRadius < 0)
		{
			UKismetSystemLibrary::LineTraceSingle(GetWorld(), CurrentLocation, TargetLocation,
				UEngineTypes::ConvertToTraceType(BULLET_TRACE_CHANNEL),true, IgnoredActors,
				DrawDebugType, HitResult, true, FLinearColor::Red, FLinearColor::Green, MaxDebugTime);
		}
		else
		{
			UKismetSystemLibrary::SphereTraceSingle(GetWorld(), CurrentLocation, TargetLocation, BulletRadius,
			UEngineTypes::ConvertToTraceType(BULLET_TRACE_CHANNEL),true, IgnoredActors,
			DrawDebugType, HitResult, true, FLinearColor::Red, FLinearColor::Green, MaxDebugTime);
		}
		
		
		if(HitResult.IsValidBlockingHit())
		{
			GetWorldTimerManager().ClearTimer(TimerHandle_Simulation);
			EPhysicalSurface SurfaceType = SurfaceType_Default;
			
			if(HasAuthority())
			{
				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
				const FVector BulletDirection = (HitResult.ImpactPoint - CurrentLocation).GetSafeNormal();
				AController* EventInstigator = OwnerPawn->GetInstigatorController();

				//PlayImpactEffects(SurfaceType, HitResult.ImpactPoint, HitResult.ImpactNormal.GetSafeNormal());
				

				// ENSURE server does damage
				if(OwnerPawn->HasAuthority())
				{
					int Damage = BulletDamage;
					switch (DamageTypeToApply)
					{
					case EDamageTypeCustom::PointDamage:
						UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), Damage, BulletDirection, HitResult, EventInstigator, OwnerPawn, DamageTypeClass);
						break;
					case EDamageTypeCustom::RadialDamage:
						UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), BulletDamage, BulletDamage/4,
							HitResult.ImpactPoint, DamageInnerRadius, DamageOuterRadius, DamageFallOff, DamageTypeClass,
							IgnoredActors, OwnerPawn, EventInstigator, ECC_Visibility);
						RadialForceComponent->FireImpulse();
						
						if(ShowDebug)
						{
							DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, DamageInnerRadius, 8, FColor::Red, false, MaxDebugTime);
							DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, DamageOuterRadius, 8, FColor::Orange, false, MaxDebugTime);
						}
						break;
					case EDamageTypeCustom::PointDamageWithPush:
						if(ADefaultPawn* HitPawn = Cast<ADefaultPawn>(HitResult.GetActor()))
						{
							if (UFloatingPawnMovement* MovementComponent = Cast<UFloatingPawnMovement>(HitPawn->GetMovementComponent()))
							{
								// Calculate the push force direction and apply it to the velocity
								const FVector BulletVelocityDirection = CurrentVelocity.GetSafeNormal();
								MovementComponent->Velocity += BulletVelocityDirection * BulletPushForce;
							}
						}
						UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), Damage, BulletDirection, HitResult, EventInstigator, OwnerPawn, DamageTypeClass);
						break;
					}
				}

				
				
			}
			
			SetActorLocation(HitResult.Location);
			SetActorRotation(CurrentVelocity.Rotation());

			StopSimulation();
			return;
		}

		//move bullet
		CurrentLocation = TargetLocation;
		OldLocation = CurrentLocation;
		
		SetActorLocation(CurrentLocation);
		//SetActorLocation(UKismetMathLibrary::VInterpTo(GetActorLocation(), CurrentLocation, timeTick, 10));
		SetActorRotation(CurrentVelocity.Rotation());
		
		//***********************************************//
	}
}

void ASPBulletBase::StopSimulation()
{
	PrimaryActorTick.bCanEverTick = false;
	GetWorldTimerManager().ClearTimer(TimerHandle_Simulation);
	bHasPendingDestroy = true;
	SceneRootComponent->SetVisibility(false, true);
	SetLifeSpan(1.0f);
}

float ASPBulletBase::GetSimulationTimeStep(float RemainingTime, int32 Iterations) const
{
	static uint32 s_WarningCount = 0;
	if (RemainingTime > MaxSimulationTimeStep)
	{
		if (Iterations < MaxSimulationIterations)
		{
			// Subdivide moves to be no longer than MaxSimulationTimeStep seconds
			RemainingTime = FMath::Min(MaxSimulationTimeStep, RemainingTime * 0.5f);
		}
		else
		{
			// If this is the last iteration, just use all the remaining time. This is usually better than cutting things short, as the simulation won't move far enough otherwise.
			// Print a throttled warning.
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if ((s_WarningCount++ < 100) || (GFrameCounter & 15) == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("APSBulletBase::GetSimulationTimeStep() - Max iterations %d hit while remaining time %.6f > MaxSimulationTimeStep (%.3f) for '%s'"), MaxSimulationIterations, RemainingTime, MaxSimulationTimeStep, *GetNameSafe(OwnerPawn));
			}
#endif
		}
	}

	// no less than MIN_TICK_TIME (to avoid potential divide-by-zero during simulation).
	return FMath::Max(MIN_TICK_TIME_BULLET, RemainingTime);
}

TArray<AActor*> ASPBulletBase::GetIgnoredActors() const
{
	AActor* Self = const_cast<ASPBulletBase*>(this);
	
	TArray<AActor*> OwnerChildren;
	OwnerPawn->GetAllChildActors(OwnerChildren);
	OwnerChildren.Add(OwnerPawn);
	OwnerChildren.Add(Self);

	return OwnerChildren;
}

