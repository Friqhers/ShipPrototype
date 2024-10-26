// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SPShip.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/SPHealthComponent.h"
#include "Core/Weapon/SPWeaponBase.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


ASPShip::ASPShip()
{
	PrimaryActorTick.bCanEverTick = true;
	bAddDefaultMovementBindings = false;
	if(GetMeshComponent())
	{
		GetMeshComponent()->bOwnerNoSee = false;
	}

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->TargetArmLength = 600.0f;
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->CameraLagSpeed = 20.0f;
	SpringArmComponent->SetupAttachment(RootComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	WeaponBase = CreateDefaultSubobject<USPWeaponBase>(TEXT("WeaponBase"));
	WeaponBase->SetupAttachment(GetMeshComponent());

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(WeaponBase);

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	
	MovementComponent->bConstrainToPlane = true;
	MovementComponent->SetPlaneConstraintNormal(FVector(1,0,0));

	HealthComponent = CreateDefaultSubobject<USPHealthComponent>(TEXT("HealthComponent"));

	bDied = false;
}

void ASPShip::BeginPlay()
{
	Super::BeginPlay();
	if(HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ASPShip::OnHealthChanged);
	}
}

void ASPShip::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		// float MouseX, MouseY;
		// PC->GetMousePosition(MouseX,MouseY);
		FVector WorldLocation, WorldDirection;
		if(PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			FVector TraceEnd;
			// Calculate the intersection of the world direction with the plane X = 0
			// Using parametric equation of the line: P = WorldLocation + t * WorldDirection
			if (WorldDirection.X != 0) // Avoid division by zero
			{
				const float PlaneX = 0.0f;
				const float T = (PlaneX - WorldLocation.X) / WorldDirection.X; // Solve for t where X = 0
				FVector SpawnLocation = WorldLocation + (T * WorldDirection); // Calculate the final location
				// Set the X value to 0 explicitly to ensure it's on the YZ plane
				SpawnLocation.X = PlaneX;
				TraceEnd = SpawnLocation;

				FVector WeaponLocation = WeaponBase->GetComponentLocation();
				WeaponLocation.X = 0;

				FVector ShipLocation = GetActorLocation();
				ShipLocation.X = 0;
				
				
				//const FVector Direction = (TraceEnd - WeaponLocation).GetSafeNormal();
				const FVector Direction = (TraceEnd - ShipLocation).GetSafeNormal();
				if (bShowDebug)
				{
					DrawDebugLine(GetWorld(), WeaponBase->GetComponentLocation(),
					              WeaponBase->GetComponentLocation() + (Direction * FVector::Distance(
						              TraceEnd, WeaponLocation)),
					              FColor::Purple, false, 0.0f);
				}

				const FRotator targetRot = Direction.Rotation();


				//WeaponBase->SetWorldRotation(targetRot);
				GetMeshComponent()->SetWorldRotation(targetRot);
			}
		}
	}

	UFloatingPawnMovement* FloatingPawnMovementComponent = Cast<UFloatingPawnMovement>(GetMovementComponent());
	if(!FloatingPawnMovementComponent) return;
	
	// auto move up when there is no player input
	if(bAutoMoveUp && FloatingPawnMovementComponent->GetLastInputVector().IsNearlyZero(0.01f))
	{
		if (bShowDebug)
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + FVector::UpVector * 300, FColor::Yellow,
			              false, 0.0f, 0, 2);
		}
		
		const FVector CurrentVelocity = FloatingPawnMovementComponent->Velocity;
		const float TargetZVelocity = UKismetMathLibrary::FInterpTo(CurrentVelocity.Z, AutoMoveUpSpeed, DeltaSeconds,
			FloatingPawnMovementComponent->Acceleration);
		FloatingPawnMovementComponent->Velocity.Z = TargetZVelocity;
	}
}

void ASPShip::SetupPlayerInputComponent(UInputComponent* InInputComponent)
{
	//Super::SetupPlayerInputComponent(InInputComponent);
	
	//InInputComponent->BindAxis("MoveForward", this, &ADefaultPawn::MoveForward);
	InInputComponent->BindAxis("MoveRight", this, &ADefaultPawn::MoveRight);
	InInputComponent->BindAxis("MoveUp", this, &ASPShip::MoveUp);
	
	InInputComponent->BindAction("Fire", IE_Pressed, this, &ASPShip::StartFire);
	InInputComponent->BindAction("Fire", IE_Released, this, &ASPShip::StopFire);
}
void ASPShip::MoveUp(float Val)
{
	if (Val != 0.f)
	{
		AddMovementInput(FVector::UpVector, Val);
	}
}

void ASPShip::OnHealthChanged(USPHealthComponent* InHealthComp, float Health, float HealthDelta,
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if(Health <= 0.0f && bDied != true)
	{
		bDied = true;
		MovementComponent->StopMovementImmediately();
		DetachFromControllerPendingDestroy();
		Destroy();

		// if(UWorld* WorldRef = GetWorld())
		// {
		// 	UWidgetLayoutLibrary::RemoveAllWidgets(WorldRef);
		// }
	}
}


void ASPShip::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if(APlayerController* PlayerController = Cast<APlayerController>(NewController))
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
		//UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController, nullptr, EMouseLockMode::DoNotLock,
		//	true, false);
	}

	if(WeaponBase)
	{
		WeaponBase->InitWeapon(NewController);
	}
}

void ASPShip::StartFire()
{
	if(WeaponBase)
	{
		WeaponBase->StartFire();
	}
}

void ASPShip::StopFire()
{
	if(WeaponBase)
	{
		WeaponBase->StopFire();
	}
}
