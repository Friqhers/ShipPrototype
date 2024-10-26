// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPBulletBase.generated.h"

class UProjectileMovementComponent;
class URadialForceComponent;

UENUM()
enum class EDamageTypeCustom : uint8
{
	RadialDamage, // does explosion logic on hit
	PointDamage, // apply damage to hit target
	PointDamageWithPush // apply damage to hit target and apply push force
};

UCLASS()
class SHIPPROTOTYPE_API ASPBulletBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPBulletBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URadialForceComponent* RadialForceComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float BulletRadius;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	int InitialVelocity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float GravityScale = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float AirResistance = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	int BulletDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float BulletPushForce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	EDamageTypeCustom DamageTypeToApply;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet")
	TSubclassOf<UDamageType> DamageTypeClass;

	// applied for radial damage bullet types
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float DamageInnerRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float DamageOuterRadius;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	float DamageFallOff = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	int MaxSimTime = 5;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, AdvancedDisplay, meta=(ClampMin="1", ClampMax="25", UIMin="1", UIMax="25"), Category = "Bullet")
	int32 MaxSimulationIterations = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, AdvancedDisplay, meta=(ClampMin="0.0166", ClampMax="0.50", UIMin="0.0166", UIMax="0.50"), Category = "Bullet")
	float MaxSimulationTimeStep = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	bool ShowDebug = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet")
	int MaxDebugTime = 1.5f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	UPROPERTY()
	APawn* OwnerPawn;
	UPROPERTY()
	TArray<AActor*> IgnoredActors;
	
	FTimerHandle TimerHandle_Simulation;
	FVector CurrentVelocity;
	FVector CurrentLocation;
	FVector OldLocation;
	float AdjustedGravity;
	bool bHasPendingDestroy;
private:
	void StopSimulation();

	virtual float GetSimulationTimeStep(float RemainingTime, int32 Iterations) const;
	TArray<AActor*> GetIgnoredActors() const;
	
	static const float MIN_TICK_TIME_BULLET;

};
