// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Weapon/SPWeaponBase.h"

#include "Core/SPEnemyBase.h"
#include "Core/SPShip.h"
#include "Core/Weapon/SPBulletBase.h"
#include "Kismet/KismetMathLibrary.h"

USPWeaponBase::USPWeaponBase()
{
	bIsAutomatic = true;
	RateOfFire = 600;
	LastFireTime = 0;
	bIsFiring = false;
	FireType = EFireType::Single;
}

void USPWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void USPWeaponBase::Fire()
{
	if(!OwnerPawn)
	{
		StopFire();
		return;
	}

	if(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds > 0)
	{
		return;
	}
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = OwnerPawn;
	SpawnParameters.Owner = OwnerPawn;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

	//@TODO: maybe use object pooling?
	switch (FireType)
	{
	case EFireType::Single:
		{
			const FVector BulletLocation = MuzzleLocation ? MuzzleLocation->GetComponentLocation() : GetComponentLocation();
			ASPBulletBase* BulletActor = GetWorld()->SpawnActor<ASPBulletBase>(BulletClass, BulletLocation, GetComponentRotation(), SpawnParameters);
			break;
		}
	case EFireType::Double:
		{
			const FVector BulletLocation = MuzzleLocation ? MuzzleLocation->GetComponentLocation() : GetComponentLocation();
			FTransform BulletTransform;
			BulletTransform.SetLocation(BulletLocation);
			BulletTransform.SetRotation(GetComponentRotation().Quaternion());
			
			ASPBulletBase* FirstBullet = GetWorld()->SpawnActorDeferred<ASPBulletBase>(BulletClass, BulletTransform, SpawnParameters.Owner,
				SpawnParameters.Instigator, SpawnParameters.SpawnCollisionHandlingOverride, SpawnParameters.TransformScaleMethod);
			ASPBulletBase* SecondBullet = GetWorld()->SpawnActorDeferred<ASPBulletBase>(BulletClass, BulletTransform, SpawnParameters.Owner,
				SpawnParameters.Instigator, SpawnParameters.SpawnCollisionHandlingOverride, SpawnParameters.TransformScaleMethod);

			if(!FirstBullet || !SecondBullet) break;

			const float Offset = FirstBullet->BulletRadius > 0 ? FirstBullet->BulletRadius/2 : 0.5f;
			FirstBullet->AddActorLocalOffset(FVector(0,0,-Offset));
			SecondBullet->AddActorLocalOffset(FVector(0,0,Offset));

			FirstBullet->FinishSpawning(FirstBullet->GetActorTransform());
			SecondBullet->FinishSpawning(SecondBullet->GetActorTransform());
			break;
		}
	case EFireType::Triple:
		{
			const FVector BulletLocation = MuzzleLocation ? MuzzleLocation->GetComponentLocation() : GetComponentLocation();
			FTransform BulletTransform;
			BulletTransform.SetLocation(BulletLocation);
			BulletTransform.SetRotation(GetComponentRotation().Quaternion());
			
			ASPBulletBase* FirstBullet = GetWorld()->SpawnActorDeferred<ASPBulletBase>(BulletClass, BulletTransform, SpawnParameters.Owner,
				SpawnParameters.Instigator, SpawnParameters.SpawnCollisionHandlingOverride, SpawnParameters.TransformScaleMethod);
			ASPBulletBase* SecondBullet = GetWorld()->SpawnActorDeferred<ASPBulletBase>(BulletClass, BulletTransform, SpawnParameters.Owner,
				SpawnParameters.Instigator, SpawnParameters.SpawnCollisionHandlingOverride, SpawnParameters.TransformScaleMethod);
			ASPBulletBase* ThirdBullet = GetWorld()->SpawnActorDeferred<ASPBulletBase>(BulletClass, BulletTransform, SpawnParameters.Owner,
				SpawnParameters.Instigator, SpawnParameters.SpawnCollisionHandlingOverride, SpawnParameters.TransformScaleMethod);

			if(!FirstBullet || !SecondBullet || !ThirdBullet) break;

			const float Offset = FirstBullet->BulletRadius > 0 ? FirstBullet->BulletRadius : 1.0f;
			FirstBullet->AddActorLocalOffset(FVector(0,0,-Offset));
			ThirdBullet->AddActorLocalOffset(FVector(0,0,Offset));

			FirstBullet->FinishSpawning(FirstBullet->GetActorTransform());
			SecondBullet->FinishSpawning(SecondBullet->GetActorTransform());
			ThirdBullet->FinishSpawning(ThirdBullet->GetActorTransform());
			break;
		}
	}

	
	LastFireTime = GetWorld()->TimeSeconds;
}

void USPWeaponBase::StartFire()
{
	const float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Fire, this, &USPWeaponBase::Fire,
	                                       TimeBetweenShots, bIsAutomatic, FirstDelay);
	
	bIsFiring = true;
}

void USPWeaponBase::StopFire()
{
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Fire);
}

float USPWeaponBase::GetFireCooldownSeconds()
{
	return FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
}

void USPWeaponBase::InitWeapon(const AController* PossessedController)
{
	OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn && PossessedController)
	{
		TimeBetweenShots = 60 / RateOfFire;
		bCanFire = true;
		bIsAIWeapon = PossessedController->IsPlayerController();

		if(bIsAIWeapon)
		{
			if(const ASPEnemyBase* EnemyBase = Cast<ASPEnemyBase>(OwnerPawn))
			{
				MuzzleLocation = EnemyBase->MuzzleLocation;
			}
		}
		else
		{
			if(const ASPShip* Ship = Cast<ASPShip>(OwnerPawn))
			{
				MuzzleLocation = Ship->MuzzleLocation;
			}
		}
	}
}
