// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "SPShip.generated.h"

class UWidgetComponent;
class USPWeaponBase;
class UCameraComponent;
class USpringArmComponent;
class USPHealthComponent;
/**
 * 
 */
UCLASS()
class SHIPPROTOTYPE_API ASPShip : public ADefaultPawn
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USPWeaponBase* WeaponBase;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* MuzzleLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USPHealthComponent* HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* HealthBarWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float WeaponInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float bAutoMoveUp = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float AutoMoveUpSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bShowDebug=false;
private:
	bool bDied;

	void MoveUp(float Val);
	
	UFUNCTION()
	virtual void OnHealthChanged(USPHealthComponent* InHealthComp, float Health, float HealthDelta,
		const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
public:
	ASPShip();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
public:
	void StartFire();
	void StopFire();
	
};
