// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SPEnemyBase.h"

#include "Components/WidgetComponent.h"
#include "Core/SPHealthComponent.h"
#include "Core/Weapon/SPWeaponBase.h"
#include "Core/SPShip.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASPEnemyBase::ASPEnemyBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bAddDefaultMovementBindings = false;
	if(GetMeshComponent())
	{
		GetMeshComponent()->bOwnerNoSee = false;
	}
	MovementComponent->bConstrainToPlane = true;
	MovementComponent->SetPlaneConstraintNormal(FVector(1,0,0));
	//MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	//RootComponent = MeshComponent;

	WeaponBase = CreateDefaultSubobject<USPWeaponBase>(TEXT("WeaponBase"));
	WeaponBase->SetupAttachment(GetMeshComponent());

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(WeaponBase);

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);

	HealthComponent = CreateDefaultSubobject<USPHealthComponent>(TEXT("HealthComponent"));

	EnemyState = EEnemyState::None;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bDied = false;
}

// Called when the game starts or when spawned
void ASPEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	FloatingPawnMovement = Cast<UFloatingPawnMovement>(MovementComponent);
	// TargetPlayer = Cast<ASPShip>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	// if(TargetPlayer)
	// {
	// 	EnemyState = (TargetPlayer->GetDistanceTo(this) <= FireRange)
	// 		             ? EEnemyState::CircleAroundPlayer
	// 		             : EEnemyState::FlyToPlayer;
	// }
	// else
	// {
	// 	EnemyState = EEnemyState::None;
	// }

	TraceType = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	GetActorBounds(true, Origin, Extent);

	
	if(HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ASPEnemyBase::OnHealthChanged);
	}
}

// Called every frame
void ASPEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TargetPlayer = Cast<ASPShip>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if(!TargetPlayer)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_RandomPositionOnCircle);
		return;
	}


	switch (EnemyState)
	{
	case EEnemyState::FlyToPlayer:
		{
			MoveLocation = TargetPlayer->GetActorLocation();
			
			// Calculate the seek force toward the target
			const FVector SeekForce = CalculateSeekForce();

			// Calculate the avoidance force to avoid obstacles
			const FVector AvoidanceForce = CalculateAvoidanceForce();

			// Combine the forces
			const FVector TotalForce = SeekForce + AvoidanceForce;
			
			// Update velocity with the total force
			FloatingPawnMovement->Velocity += (TotalForce * FloatingPawnMovement->TurningBoost) * DeltaTime;
			FloatingPawnMovement->Velocity = FloatingPawnMovement->Velocity.GetClampedToMaxSize(MaxVelocity);
	
			// Move the pawn
			AddMovementInput(FloatingPawnMovement->Velocity * DeltaTime);

			// check if in fire range
			if(FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation()) <= FireRange)
			{
				GetWorldTimerManager().ClearTimer(TimerHandle_RandomPositionOnCircle);
				GetWorldTimerManager().SetTimer(TimerHandle_RandomPositionOnCircle,this, &ASPEnemyBase::FindSuitablePositionAroundPlayer,
					MoveFailSeconds,true,0);
				EnemyState = EEnemyState::CircleAroundPlayer;
			}
			break;
		}
	case EEnemyState::CircleAroundPlayer:
		{
			//try fire
			if(WeaponBase)
				WeaponBase->Fire();

			// Calculate the seek force toward the target
			const FVector SeekForce = CalculateSeekForce();

			// Calculate the avoidance force to avoid obstacles
			const FVector AvoidanceForce = CalculateAvoidanceForce();

			// Combine the forces
			const FVector TotalForce = SeekForce + AvoidanceForce;
			
			// Update velocity with the total force
			FloatingPawnMovement->Velocity += (TotalForce * FloatingPawnMovement->TurningBoost) * DeltaTime;
			FloatingPawnMovement->Velocity = FloatingPawnMovement->Velocity.GetClampedToMaxSize(MaxVelocity);

			// Move the pawn
			AddMovementInput(FloatingPawnMovement->Velocity * DeltaTime);

			if(MoveLocation.Equals(GetActorLocation(), Extent.Size()))
			{
				GetWorldTimerManager().ClearTimer(TimerHandle_RandomPositionOnCircle);
				GetWorldTimerManager().SetTimer(TimerHandle_RandomPositionOnCircle,this, &ASPEnemyBase::FindSuitablePositionAroundPlayer,
					MoveFailSeconds,true,0);
			}
			
			//check if far
			if(FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation()) > FireRange)
			{
				if(WeaponBase)
					WeaponBase->StopFire();
				GetWorldTimerManager().ClearTimer(TimerHandle_RandomPositionOnCircle);
				EnemyState = EEnemyState::FlyToPlayer;
			}
			break;
		}
	case EEnemyState::None:
		{
			EnemyState = (TargetPlayer->GetDistanceTo(this) <= FireRange)
						 ? EEnemyState::CircleAroundPlayer
						 : EEnemyState::FlyToPlayer;
		}
		break;
	}


	// Rotate weapon towards player
	if(WeaponBase && TargetPlayer)
	{
		const FVector Direction = (TargetPlayer->GetActorLocation() - WeaponBase->GetComponentLocation()).GetSafeNormal();
		if(bShowDebug)
		{
			DrawDebugLine(GetWorld(), WeaponBase->GetComponentLocation(),
			              WeaponBase->GetComponentLocation() + (Direction * FVector::Distance(
				              TargetPlayer->GetActorLocation(), WeaponBase->GetComponentLocation())),
			              FColor::Orange, false, 0.0f);
		}
		FRotator targetRot = Direction.Rotation();
		WeaponBase->SetWorldRotation(targetRot);
	}
}

void ASPEnemyBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if(WeaponBase)
	{
		WeaponBase->InitWeapon(NewController);
	}
}

void ASPEnemyBase::OnHealthChanged(USPHealthComponent* InHealthComp, float Health, float HealthDelta,
                                   const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if(Health <= 0.0f && bDied != true)
	{
		bDied = true;
		MovementComponent->StopMovementImmediately();
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.5f);
	}
}

FVector ASPEnemyBase::CalculateSeekForce()
{
	const FVector DesiredVelocity = (MoveLocation - GetActorLocation()).GetSafeNormal() * MaxVelocity;

	// Calculate the steering force by subtracting current velocity from desired velocity
	return (DesiredVelocity - MovementComponent->Velocity) * SteeringStrength;

}

FVector ASPEnemyBase::CalculateAvoidanceForce()
{
	FVector CurrentLocation = GetActorLocation();
	FVector MoveDirection = MovementComponent->Velocity.GetSafeNormal();
	
	FVector TraceEnd = GetActorLocation() + (MoveDirection * Extent.Size()*2);
	
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	IgnoredActors.Add(TargetPlayer);

	FHitResult HitResult;
	UKismetSystemLibrary::BoxTraceSingle(GetWorld(), GetActorLocation(),
		TraceEnd, Extent,
		FRotator::ZeroRotator,TraceType,
		false, IgnoredActors,
		(bShowDebug ? EDrawDebugTrace::ForDuration: EDrawDebugTrace::None),
		HitResult,
		true,FLinearColor::Red,
		FLinearColor::Green, 0.0f);

	if(HitResult.bBlockingHit)
	{
		FVector ObstacleLocation = HitResult.ImpactPoint;
		FVector AvoidanceDirection = (CurrentLocation - ObstacleLocation).GetSafeNormal();
        
		// Apply the avoidance force
		return AvoidanceDirection * AvoidanceStrength;
	}
	return FVector::ZeroVector;
}

void ASPEnemyBase::FindSuitablePositionAroundPlayer()
{
	FHitResult HitResult;
	int counter = 0;
	
	while (HitResult.bBlockingHit || counter == 0)
	{
		FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();
		RandomDirection.X = 0;
		RandomDirection.Normalize();
		MoveLocation = TargetPlayer->GetActorLocation() + (RandomDirection * FireRange);

		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		IgnoredActors.Add(TargetPlayer);

		// trace from enemy to move location to see if we can get there
		UKismetSystemLibrary::BoxTraceSingle(GetWorld(), GetActorLocation(),
		                                     MoveLocation, Extent,
		                                     FRotator::ZeroRotator, TraceType,
		                                     false, IgnoredActors,
		                                     (bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None),
		                                     HitResult,
		                                     true, FLinearColor::Red,
		                                     FLinearColor::Green, 0.0f);

		if (bShowDebug)
		{
			DrawDebugLine(GetWorld(), TargetPlayer->GetActorLocation(),
				HitResult.bBlockingHit ? HitResult.ImpactPoint: MoveLocation,
				HitResult.bBlockingHit ? FColor::Green : FColor::Red, false, 0.5f);
		}



		counter++;
		if (counter > 20)
			break;
	}

}















// void ASPEnemyBase::FindTargetMoveLocation()
// {
// 	TArray<FHitResult> HitResults = GetHitsAroundActor();
// 	FVector TargetLocation = FVector::ZeroVector;
// 	for (const FHitResult HitResult : HitResults)
// 	{
// 		if(!HitResult.bBlockingHit)
// 		{
// 			const float oldDistance = FVector::Distance(TargetLocation, TargetPlayer->GetActorLocation());
// 			const float currentDistance = FVector::Distance(HitResult.TraceEnd, TargetPlayer->GetActorLocation());
//
// 			//select the closest point
// 			if(currentDistance < oldDistance)
// 			{
// 				TargetLocation = HitResult.TraceEnd;
// 			}
// 		}
// 	}
//
// 	MoveLocation = TargetLocation;
// 	if(bShowDebug)
// 		DrawDebugLine(GetWorld(), GetActorLocation(), MoveLocation, FColor::Purple, false, TraceInterval);
// }
//
// TArray<FHitResult> ASPEnemyBase::GetHitsAroundActor()
// {
// 	TArray<FHitResult> HitResults;
// 	
// 	TArray<FVector> TraceDirections = {
// 		FVector(0,0,1), FVector(0,0,-1), //up, down
// 		FVector(0,1,0), FVector(0,-1,0), // right, left
// 		FVector(0,0.5f,0.5f), FVector(0,-0.5,0.5f), // up-right, up-left
// 		FVector(0,0.5f,-0.5f), FVector(0,-0.5,-0.5f), // down-right, down-left
// 	};
// 	
// 	TArray<AActor*> IgnoredActors;
// 	IgnoredActors.Add(this);
//
// 	for (FVector Direction : TraceDirections)
// 	{
// 		FHitResult HitResult;
// 		FVector TraceEnd = GetActorLocation() + (Direction * Extent.Size()*2);
// 		// Convert ECollisionChannel to ETraceTypeQuery
// 		ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
// 		
// 		UKismetSystemLibrary::BoxTraceSingle(GetWorld(), GetActorLocation(),
// 			TraceEnd, Extent,
// 			FRotator::ZeroRotator,TraceType,
// 			false, IgnoredActors,
// 			(bShowDebug ? EDrawDebugTrace::ForDuration: EDrawDebugTrace::None),
// 			HitResult,
// 			true,FLinearColor::Red,
// 			FLinearColor::Green, TraceInterval);
//
// 		
// 		HitResults.Add(HitResult);
// 	}
// 	return HitResults;
// }