// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SPHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USPHealthComponent*, InHealthComp,
	float, Health, float, HealthDelta, const class UDamageType*, DamageType,
	class AController*, InstigatedBy, AActor*, DamageCauser);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnArmorChangedSignature, USPHealthComponent*, InHealthComp,
	float, Armor, float, ArmorDelta, const class UDamageType*, DamageType,
	class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHIPPROTOTYPE_API USPHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USPHealthComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
	int TeamID;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
	float MaxHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultArmor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
	float MaxArmor;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
	bool bHasArmor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HealthComponent")
	bool bCanRegenHealth = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,Category = "HealthComponent")
	float HealthRegenCooldown = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,Category = "HealthComponent")
	float HealthRegenSpeed = 2.0f;
public:
	bool bIsDead;

	UPROPERTY(BlueprintReadOnly, Category = "HealthComponent")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "HealthComponent")
	float CurrentArmor;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	static bool IsFriendly(const AActor* ActorA, const AActor* ActorB);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnArmorChangedSignature OnArmorChanged;	
private:
	FTimerHandle TimerHandle_HealthRegenCooldown;
	UFUNCTION()
	void OnHealthRegenCooldown();
		
};
