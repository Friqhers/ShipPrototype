// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/DefaultPawn.h"
#include "SPEnemyBase.generated.h"

class USPHealthComponent;
class UFloatingPawnMovement;
class ASPShip;
class USPWeaponBase;
class UWidgetComponent;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	FlyToPlayer,
	CircleAroundPlayer,
	None
};

UCLASS()
class SHIPPROTOTYPE_API ASPEnemyBase : public ADefaultPawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPEnemyBase();

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	//UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USPWeaponBase* WeaponBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* MuzzleLocation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USPHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* HealthBarWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Settings")
	float FireRange = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Settings")
	float TraceInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Settings")
	float TraceDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Settings")
	float MaxVelocity = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Settings")
	bool bShowDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Settings")
	float SteeringStrength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Settings")
	float AvoidanceStrength = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Settings")
	float MoveFailSeconds = 2.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PossessedBy(AController* NewController) override;


	UFUNCTION()
	virtual void OnHealthChanged(USPHealthComponent* InHealthComp, float Health, float HealthDelta,
		const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	EEnemyState GetEnemyState(){return EnemyState;}
private:
	UPROPERTY()
	ASPShip* TargetPlayer;
	UPROPERTY()
	UFloatingPawnMovement* FloatingPawnMovement;

	EEnemyState EnemyState;
	FTimerHandle TimerHandle_RandomPositionOnCircle;
	FVector MoveLocation;
	FVector Origin, Extent;
	ETraceTypeQuery TraceType;
	bool bDied;


	void FindTargetMoveLocation();
	TArray<FHitResult> GetHitsAroundActor();

	////
	FVector CalculateSeekForce();
	FVector CalculateAvoidanceForce();
	void FindSuitablePositionAroundPlayer();
};
