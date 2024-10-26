// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "SPWeaponBase.generated.h"

class ASPBulletBase;

UENUM(BlueprintType)
enum class EFireType : uint8
{
	Single,
	Double,
	Triple
};
/**
 * 
 */
UCLASS()
class SHIPPROTOTYPE_API USPWeaponBase : public UStaticMeshComponent
{
	GENERATED_BODY()
public:
	USPWeaponBase();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float RateOfFire;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsAutomatic;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EFireType FireType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<ASPBulletBase> BulletClass;
protected:
	virtual void BeginPlay() override;
public:
	void Fire();
	void StartFire();
	void StopFire();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetFireCooldownSeconds();

	void InitWeapon(const AController* PossessedController);
private:
	UPROPERTY()
	APawn* OwnerPawn;

	UPROPERTY()
	USceneComponent* MuzzleLocation;
	
	FTimerHandle TimerHandle_Fire;
	FTimerHandle TimerHandle_StopFire;
	float TimeBetweenShots;
	float LastFireTime;
	bool bIsFiring;
	bool bCanFire;
	bool bIsAIWeapon;
};
